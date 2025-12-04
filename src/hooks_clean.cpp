// SIMPLE DX12 HOOKING - NO IMGUI DEPENDENCIES
#include <windows.h>
#include <iostream>
#include <MinHook.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include "frame_capture.h"
#include "frame_processor.h"

// Typedefs
typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
typedef void(__stdcall* ExecuteCommandLists_t)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

// Global hooks
Present_t origPresent = nullptr;
ExecuteCommandLists_t origExecuteCommandLists = nullptr;
ID3D12CommandQueue* g_pGameCommandQueue = nullptr;
bool g_hooksInitialized = false;

// Hook Present function - FRAMES INTERCEPTED
HRESULT __stdcall hkPresent(IDXGISwapChain* swapchain, UINT syncInterval, UINT flags) {
    static int frameCount = 0;
    static bool firstFrameDone = false;

    frameCount++;

    if (!firstFrameDone) {
        firstFrameDone = true;
        std::cout << "[CROSSLINK] Frame interception active! Processing frames from Nvidia GPU." << std::endl;

        // Initialize frame capture on first frame
        ID3D12Device* device = nullptr;
        if (SUCCEEDED(swapchain->GetDevice(__uuidof(ID3D12Device), (void**)&device))) {
            if (FrameCapture::Initialize(device)) {
                std::cout << "[FRAME CAPTURE] Nvidia GPU frame capture initialized." << std::endl;
            }
            device->Release();
        }
    }

    // CAPTURE AND PROCESS FRAME
    if (FrameCapture::IsInitialized() && FrameProcessor::IsRunning()) {
        FrameCapture::CaptureFromSwapChain(swapchain);

        const CapturedFrame* frame = FrameCapture::GetLatestFrame();
        if (frame && frame->frameNumber > 0) {
            FrameProcessor::QueueFrame(*frame);
        }
    }

    // Log stats every 60 frames (1 second)
    if (frameCount % 60 == 0) {
        std::cout << "[CROSSLINK] Frames processed: " << frameCount
                  << " - GPU Cross-Link active" << std::endl;
    }

    // Call original Present
    return origPresent(swapchain, syncInterval, flags);
}

// Hook ExecuteCommandLists - CAPTURE COMMAND QUEUE
void __stdcall hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
    // CAPTURE GAME'S COMMAND QUEUE ON FIRST CALL
    if (!g_pGameCommandQueue) {
        g_pGameCommandQueue = pCommandQueue;
        g_pGameCommandQueue->AddRef();
        std::cout << "[QUEUE CAPTURE] Nvidia GPU command queue captured successfully." << std::endl;
        std::cout << "[CROSSLINK] Ready for Cross-GPU frame generation!" << std::endl;
    }

    // Call original function
    origExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

// GET PRESENT ADDRESS VIA VTLLable SCANNING
void* GetDX12PresentAddress() {
    std::cout << "[VTABLE SCAN] Locating DX12 Present function..." << std::endl;

    // Create dummy objects to scan VTable
    HMODULE d3d12 = LoadLibraryA("d3d12.dll");
    HMODULE dxgi = LoadLibraryA("dxgi.dll");

    if (!d3d12 || !dxgi) {
        std::cout << "[ERROR] Failed to load graphics libraries!" << std::endl;
        return nullptr;
    }

    // Function pointers
    auto pD3D12CreateDevice = (HRESULT(WINAPI*)(IUnknown*, UINT, REFIID, void**))GetProcAddress(d3d12, "D3D12CreateDevice");
    auto pCreateDXGIFactory1 = (HRESULT(WINAPI*)(REFIID, void**))GetProcAddress(dxgi, "CreateDXGIFactory1");

    // Dummy window
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = TEXT("DummyCrossLink");
    RegisterClassEx(&wc);
    HWND dummy = CreateWindowA("DummyCrossLink", "", 0, 0, 0, 100, 100, NULL, NULL, wc.hInstance, NULL);

    // Create dummy device and swapchain
    ID3D12Device* device = nullptr;
    ID3D12CommandQueue* cmdQueue = nullptr;
    IDXGIFactory4* factory = nullptr;
    IDXGISwapChain1* swapChain = nullptr;

    if (SUCCEEDED(pD3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&device))) {
        // Create command queue
        D3D12_COMMAND_QUEUE_DESC cqDesc = { D3D12_COMMAND_LIST_TYPE_DIRECT };
        device->CreateCommandQueue(&cqDesc, __uuidof(ID3D12CommandQueue), (void**)&cmdQueue);

        // Create factory and swapchain
        pCreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&factory);

        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.Width = 800;
        scDesc.Height = 600;
        scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.BufferCount = 2;
        scDesc.SampleDesc.Count = 1;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

        factory->CreateSwapChainForHwnd((IUnknown*)cmdQueue, dummy, &scDesc, nullptr, nullptr, &swapChain);

        // SCAN VTLLable FOR PRESENT FUNCTION
        void** vtable = *(void***)swapChain;
        void* presentAddr = vtable[8]; // Present at index 8

        std::cout << "[VTABLE SUCCESS] Found Present at: 0x" << std::hex << (uintptr_t)presentAddr << std::dec << std::endl;

        // Cleanup
        swapChain->Release();
        factory->Release();
        cmdQueue->Release();
        device->Release();
        DestroyWindow(dummy);

        return presentAddr;
    }

    DestroyWindow(dummy);
    std::cout << "[VTABLE ERROR] Failed to create dummy graphics objects!" << std::endl;
    return nullptr;
}

// HOOK INITIALIZATION
bool InitializeHooks() {
    if (g_hooksInitialized) return true;

    std::cout << "[CROSSLINK] Initializing DX12 hooks for Borderlands 3..." << std::endl;

    // Initialize MinHook
    if (MH_Initialize() != MH_OK) {
        std::cout << "[ERROR] Failed to initialize MinHook!" << std::endl;
        return false;
    }

    // Get Present function address
    void* presentTarget = GetDX12PresentAddress();
    if (!presentTarget) {
        std::cout << "[ERROR] Failed to locate Present function!" << std::endl;
        return false;
    }

    // Create Present hook
    if (MH_CreateHook(presentTarget, hkPresent, (LPVOID*)&origPresent) != MH_OK) {
        std::cout << "[ERROR] Failed to create Present hook!" << std::endl;
        return false;
    }

    // Enable Present hook
    if (MH_EnableHook(presentTarget) != MH_OK) {
        std::cout << "[ERROR] Failed to enable Present hook!" << std::endl;
        return false;
    }

    g_hooksInitialized = true;
    std::cout << "[SUCCESS] CrossLink DX12 hooks active - Frame generation ready!" << std::endl;
    return true;
}

void ShutdownHooks() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if (g_pGameCommandQueue) {
        g_pGameCommandQueue->Release();
        g_pGameCommandQueue = nullptr;
    }

    g_hooksInitialized = false;
    std::cout << "[SHUTDOWN] CrossLink hooks disabled." << std::endl;
}
