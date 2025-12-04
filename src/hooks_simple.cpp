// Universal DX12 VTable Hooking Implementation with CommandQueue Capture
// REVOLUTIONARY FUNKTION: Present via VTable finden, um Anti-Cheat zu umgehen
#include "hooks.h"
#include <Windows.h>
#include <d3d12.h>
#include <dxgi.h>
#include <dxgi1_4.h>
#include <dxgi1_6.h>
#include <iostream>
#include <MinHook.h>
#include "imgui_manager.h"
#include "frame_capture.h"
#include "frame_processor.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// VTable Index für Present
#define IDXGISWAPCHAIN_PRESENT_INDEX 8

// Structure to hold both function addresses
struct HookedFunctions {
    void* presentAddr;
    void* executeCommandListsAddr;
};

// Globals
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D12Device* g_pd3dDevice = nullptr;
static bool g_HookInitialized = false;
static bool g_FirstFrame = true;
static HookedFunctions g_HookedFunctions = { nullptr, nullptr };
ID3D12CommandQueue* g_pGameCommandQueue = nullptr;

// Typedefs
typedef HRESULT(WINAPI* PresentFunc)(IDXGISwapChain*, UINT, UINT);
typedef void(__stdcall* ExecuteCommandListsFunc)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

static PresentFunc OriginalPresent = nullptr;
static ExecuteCommandListsFunc OriginalExecuteCommandLists = nullptr;

// CRITICAL: Hook ExecuteCommandLists to capture the game's command queue
void __stdcall hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
    // CAPTURE THE GAME'S COMMAND QUEUE - PREPARATION FOR IMGUI RENDERING
    if (!g_pGameCommandQueue) {
        g_pGameCommandQueue = pCommandQueue;
        g_pGameCommandQueue->AddRef();
        std::cout << "[QUEUE CAPTURED] Game's DX12 command queue captured - ready for ImGui rendering!" << std::endl;
    }

    // Call original function
    OriginalExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

// Dummy window procedure
LRESULT CALLBACK DummyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// FUNCTION: Get BOTH DX12 Present AND ExecuteCommandLists addresses via VTable scanning
void* GetDX12PresentAddress() {
    std::cout << "[HOOK] Creating dummy D3D12 device for VTable scanning..." << std::endl;

    try {
        // STEP 1: Create dummy window
        HINSTANCE hInstance = GetModuleHandle(NULL);
        WNDCLASSEXA wc = { 0 };
        wc.cbSize = sizeof(WNDCLASSEXA);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = DummyWndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = "DummyCrossLinkFG";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);

        if (!RegisterClassExA(&wc)) {
            std::cout << "[HOOK] Failed to register window class" << std::endl;
            return nullptr;
        }

        std::cout << "[HOOK] Dummy window class registered." << std::endl;

        // Create hidden dummy window
        HWND hwnd = CreateWindowA("DummyCrossLinkFG", "Dummy", 0, 0, 0, 800, 600, NULL, NULL, hInstance, NULL);
        if (!hwnd) {
            std::cout << "[HOOK] Failed to create dummy window" << std::endl;
            return nullptr;
        }

        std::cout << "[HOOK] Dummy window created." << std::endl;

        // STEP 2: Load D3D12 and DXGI DLLs
        HMODULE d3d12 = LoadLibraryA("d3d12.dll");
        HMODULE dxgi = LoadLibraryA("dxgi.dll");

        if (!d3d12 || !dxgi) {
            std::cout << "[HOOK] Failed to load d3d12.dll or dxgi.dll" << std::endl;
            DestroyWindow(hwnd);
            return nullptr;
        }

        auto pD3D12CreateDevice = (HRESULT(WINAPI*)(IUnknown*, UINT, REFIID, void**))GetProcAddress(d3d12, "D3D12CreateDevice");
        auto pCreateDXGIFactory1 = (HRESULT(WINAPI*)(REFIID, void**))GetProcAddress(dxgi, "CreateDXGIFactory1");

        if (!pD3D12CreateDevice || !pCreateDXGIFactory1) {
            std::cout << "[HOOK] Failed to get D3D12 or DXGI function pointers" << std::endl;
            DestroyWindow(hwnd);
            return nullptr;
        }

        // STEP 3: Create D3D12 device
        ID3D12Device* device = nullptr;
        HRESULT hr = pD3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&device);

        if (FAILED(hr) || !device) {
            std::cout << "[HOOK] Failed to create D3D12 device" << std::endl;
            DestroyWindow(hwnd);
            return nullptr;
        }

        std::cout << "[HOOK] D3D12 device created." << std::endl;

        // STEP 4: Create command queue
        ID3D12CommandQueue* commandQueue = nullptr;
        D3D12_COMMAND_QUEUE_DESC cqDesc = {};
        cqDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

        hr = device->CreateCommandQueue(&cqDesc, __uuidof(ID3D12CommandQueue), (void**)&commandQueue);
        if (FAILED(hr) || !commandQueue) {
            std::cout << "[HOOK] Failed to create command queue" << std::endl;
            device->Release();
            DestroyWindow(hwnd);
            return nullptr;
        }

        std::cout << "[HOOK] Command queue created." << std::endl;

        // STEP 5: Create DXGI factory
        IDXGIFactory4* factory = nullptr;
        hr = pCreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&factory);

        if (FAILED(hr) || !factory) {
            std::cout << "[HOOK] Failed to create DXGI factory" << std::endl;
            commandQueue->Release();
            device->Release();
            DestroyWindow(hwnd);
            return nullptr;
        }

        std::cout << "[HOOK] DXGI factory created." << std::endl;

        // STEP 6: Create temporary swapchain to extract VTable
        IDXGISwapChain* swapChain = nullptr;
        DXGI_SWAP_CHAIN_DESC1 scDesc = {};
        scDesc.Width = 800;
        scDesc.Height = 600;
        scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scDesc.Stereo = FALSE;
        scDesc.SampleDesc.Count = 1;
        scDesc.SampleDesc.Quality = 0;
        scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scDesc.BufferCount = 2;
        scDesc.Scaling = DXGI_SCALING_STRETCH;
        scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
        scDesc.Flags = 0;

        hr = factory->CreateSwapChainForHwnd((IUnknown*)commandQueue, hwnd, &scDesc, nullptr, nullptr, (IDXGISwapChain1**)&swapChain);

        if (FAILED(hr) || !swapChain) {
            std::cout << "[HOOK] Failed to create dummy swapchain" << std::endl;
            factory->Release();
            commandQueue->Release();
            device->Release();
            DestroyWindow(hwnd);
            return nullptr;
        }

        std::cout << "[HOOK] Dummy swapchain created." << std::endl;

        // STEP 7: CRITICAL - Extract function pointers from VTable
        void** vtable = *(void***)swapChain;
        void* presentFunc = vtable[8];  // Present at index 8
        void* executeCommandListsFunc = vtable[10]; // ExecuteCommandLists at index 10

        std::cout << "[HOOK] VTable scanned successfully!" << std::endl;
        std::cout << "[HOOK] Present function: 0x" << std::hex << (uintptr_t)presentFunc << std::dec << std::endl;
        std::cout << "[HOOK] ExecuteCommandLists function: 0x" << std::hex << (uintptr_t)executeCommandListsFunc << std::dec << std::endl;

        // STEP 8: Store function addresses globally
        g_HookedFunctions.presentAddr = presentFunc;
        g_HookedFunctions.executeCommandListsAddr = executeCommandListsFunc;

        // Cleanup dummy objects
        swapChain->Release();
        factory->Release();
        commandQueue->Release();
        device->Release();
        DestroyWindow(hwnd);
        UnregisterClassA("DummyCrossLinkFG", hInstance);

        std::cout << "[HOOK] VTable scanning completed successfully!" << std::endl;
        return presentFunc;

    } catch (const std::exception& e) {
        std::cout << "[HOOK] Exception during VTable scanning: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cout << "[HOOK] Unknown exception during VTable scanning." << std::endl;
        return nullptr;
    }
}

// Hooked Present function - FRAME CAPTURE & GENERATION
HRESULT __stdcall hkPresent(IDXGISwapChain* swapchain, UINT syncInterval, UINT flags) {
    static int frameCount = 0;

    if (frameCount == 0) {
        // First frame - initialize all systems
        std::cout << "[HOOK SUCCESS] hkPresent called! CrossLink FG is active in Borderlands!" << std::endl;
        std::cout << "[HOOK SUCCESS] Frame interception confirmed - ready for frame generation." << std::endl;

        // Initialize Frame Capture system
        ID3D12Device* device = nullptr;
        if (SUCCEEDED(swapchain->GetDevice(__uuidof(ID3D12Device), (void**)&device))) {
            if (FrameCapture::Initialize(device)) {
                std::cout << "[FRAME CAPTURE] Successfully initialized frame capture system!" << std::endl;
            } else {
                std::cout << "[FRAME CAPTURE ERROR] Failed to initialize frame capture!" << std::endl;
            }
            device->Release();
        }
    }

    frameCount++;

    // CRITICAL: Frame Capture - Capture game's backbuffer
    if (FrameCapture::IsInitialized()) {
        if (FrameCapture::CaptureFromSwapChain(swapchain)) {
            // Successful capture - queue for processing
            const CapturedFrame* frame = FrameCapture::GetLatestFrame();
            if (frame && frame->frameNumber > 0) {
                FrameProcessor::QueueFrame(*frame);
            }
        }
    }

    // Call FrameGenManager's OnPresent (commented for now - needs integration)
    // FrameGenManager::GetInstance().OnPresent(swapchain);

    // Log frame interception stats
    if (frameCount % 60 == 0) {
        std::cout << "[HOOK] Frames intercepted: " << frameCount << " - CrossLink FG active" << std::endl;
        if (g_pGameCommandQueue) {
            std::cout << "[QUEUE STATUS] Command queue captured - ImGui ready" << std::endl;
        } else {
            std::cout << "[QUEUE STATUS] Waiting for command queue capture..." << std::endl;
        }
        if (FrameCapture::IsInitialized()) {
            std::cout << "[FRAME CAPTURE] Active - processing frames" << std::endl;
        } else {
            std::cout << "[FRAME CAPTURE] Not initialized yet" << std::endl;
        }
    }

    // Call original Present function
    return OriginalPresent(swapchain, syncInterval, flags);
}

// Universal hook initialization - hooks BOTH Present AND ExecuteCommandLists
bool InitializeHooks() {
    std::cout << "[HOOK] Initializing Dual DX12 VTable Hooking for Borderlands..." << std::endl;

    // Initialize MinHook library
    if (MH_Initialize() != MH_OK) {
        std::cout << "[HOOK] Failed to initialize MinHook." << std::endl;
        return false;
    }

    // Get function addresses via VTable scanning
    void* presentTarget = GetDX12PresentAddress();
    if (!presentTarget || !g_HookedFunctions.executeCommandListsAddr) {
        std::cout << "[HOOK] Failed to locate Present and ExecuteCommandLists functions." << std::endl;
        return false;
    }

    // Hook Present function
    MH_STATUS presentStatus = MH_CreateHook(presentTarget, hkPresent, (LPVOID*)&OriginalPresent);
    if (presentStatus != MH_OK) {
        std::cout << "[HOOK] Failed to create Present hook. Error: " << presentStatus << std::endl;
        return false;
    }

    presentStatus = MH_EnableHook(presentTarget);
    if (presentStatus != MH_OK) {
        std::cout << "[HOOK] Failed to enable Present hook. Error: " << presentStatus << std::endl;
        return false;
    }

    // CRITICAL: Hook ExecuteCommandLists for command queue capture
    MH_STATUS executeStatus = MH_CreateHook(g_HookedFunctions.executeCommandListsAddr, hkExecuteCommandLists, (LPVOID*)&OriginalExecuteCommandLists);
    if (executeStatus != MH_OK) {
        std::cout << "[HOOK] Failed to create ExecuteCommandLists hook. Error: " << executeStatus << std::endl;
        return false;
    }

    executeStatus = MH_EnableHook(g_HookedFunctions.executeCommandListsAddr);
    if (executeStatus != MH_OK) {
        std::cout << "[HOOK] Failed to enable ExecuteCommandLists hook. Error: " << executeStatus << std::endl;
        return false;
    }

    std::cout << "[HOOK] Dual DX12 hooks initialized successfully!" << std::endl;
    std::cout << "[HOOK] Present + ExecuteCommandLists interception active" << std::endl;
    std::cout << "[HOOK] Ready for in-game ImGui overlay implementation" << std::endl;

    return true;
}

void ShutdownHooks() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    if (g_pGameCommandQueue) {
        g_pGameCommandQueue->Release();
    }

    std::cout << "[HOOK] All hooks shutdown successfully." << std::endl;
}
