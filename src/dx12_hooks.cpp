// DX12 HOOKS - Advanced ImGui Overlay System for Borderlands
#include "imgui_manager.h"
#include <MinHook.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// Globale Pointer
ID3D12CommandQueue* g_pCapturedQueue = nullptr;
IDXGISwapChain* g_pSwapChain = nullptr;
ID3D12Device* g_pDevice = nullptr;

// Overlay Rendering Ressourcen
ID3D12CommandAllocator* g_pCmdAllocators[3] = { nullptr };
ID3D12GraphicsCommandList* g_pCmdList = nullptr;
ID3D12DescriptorHeap* g_pRtvHeap = nullptr;
ID3D12Resource* g_pRenderTargets[3] = { nullptr };
D3D12_CPU_DESCRIPTOR_HANDLE g_rtvHandles[3];
UINT g_BackBufferCount = 0;

// Original Funktionen
typedef HRESULT(WINAPI* Present_t)(IDXGISwapChain*, UINT, UINT);
typedef void(WINAPI* ExecuteCommandLists_t)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

// --- HELPER: Render Targets erstellen ---
void CreateOverlayResources(IDXGISwapChain* pSwapChain) {
    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc(&desc);
    g_BackBufferCount = desc.BufferCount;

    // RTV Heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.NumDescriptors = g_BackBufferCount;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    g_pDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_pRtvHeap));

    SIZE_T rtvSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pRtvHeap->GetCPUDescriptorHandleForHeapStart();

    // Resources
    for (UINT i = 0; i < g_BackBufferCount; i++) {
        g_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&g_pCmdAllocators[i]));
        pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_pRenderTargets[i]));
        g_pDevice->CreateRenderTargetView(g_pRenderTargets[i], nullptr, rtvHandle);
        g_rtvHandles[i] = rtvHandle;
        rtvHandle.ptr += rtvSize;
    }

    // Command List
    g_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_pCmdAllocators[0], nullptr, IID_PPV_ARGS(&g_pCmdList));
    g_pCmdList->Close();
}

// --- HOOK 1: EXECUTE COMMAND LISTS (Queue Capture) ---
void WINAPI hkExecuteCommandLists(ID3D12CommandQueue* pQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
    // Wir klauen die Queue beim ersten Aufruf
    if (g_pCapturedQueue == nullptr && pQueue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
        g_pCapturedQueue = pQueue;
        ImGuiManager::SetCommandQueue(pQueue);
        std::cout << "[HOOKS] Captured CommandQueue: 0x" << std::hex << pQueue << std::dec << std::endl;
    }
    oExecuteCommandLists(pQueue, NumCommandLists, ppCommandLists);
}

// --- HOOK 2: PRESENT (Rendering) ---
HRESULT WINAPI hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    static bool init = false;
    if (!init) {
        if (SUCCEEDED(pSwapChain->GetDevice(IID_PPV_ARGS(&g_pDevice)))) {
            CreateOverlayResources(pSwapChain);
            ImGuiManager::Initialize(g_pDevice, pSwapChain);
            init = true;
        }
    }

    // Overlay Zeichnen - NUR wenn wir die Queue haben!
    if (init && g_pCapturedQueue && ImGuiManager::IsInitialized()) {
        IDXGISwapChain3* pSwapChain3 = (IDXGISwapChain3*)pSwapChain;
        UINT index = pSwapChain3->GetCurrentBackBufferIndex();

        // 1. Reset
        g_pCmdAllocators[index]->Reset();
        g_pCmdList->Reset(g_pCmdAllocators[index], nullptr);

        // 2. Barrier: Present -> RT
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = g_pRenderTargets[index];
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_pCmdList->ResourceBarrier(1, &barrier);

        // 3. Render Setup
        g_pCmdList->OMSetRenderTargets(1, &g_rtvHandles[index], FALSE, nullptr);

        // 4. ImGui
        ImGuiManager::NewFrame();
        ImGuiManager::RenderOverlay();
        ImGuiManager::Render(g_pCmdList);

        // 5. Barrier: RT -> Present
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_pCmdList->ResourceBarrier(1, &barrier);

        g_pCmdList->Close();

        // 6. EXECUTE!
        ID3D12CommandList* ppLists[] = { g_pCmdList };
        g_pCapturedQueue->ExecuteCommandLists(1, ppLists);
    }

    return oPresent(pSwapChain, SyncInterval, Flags);
}

// --- INSTALLATION (VTABLE SCANNING) ---
bool InstallDX12Hooks() {
    // Dummy Device erstellen um VTable zu lesen
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("DX"), NULL };
    RegisterClassEx(&wc);
    HWND hWnd = CreateWindow(_T("DX"), _T("DX"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720, NULL, NULL, wc.hInstance, NULL);

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ID3D12Device* d3dDevice = nullptr;
    ID3D12CommandQueue* d3dCommandQueue = nullptr;
    IDXGISwapChain* d3dSwapChain = nullptr;

    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice)))) return false;
    if (FAILED(d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&d3dCommandQueue)))) return false;

    IDXGIFactory4* factory;
    CreateDXGIFactory1(IID_PPV_ARGS(&factory));

    DXGI_SWAP_CHAIN_DESC swapDesc = {};
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.OutputWindow = hWnd;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.Windowed = TRUE;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    factory->CreateSwapChain(d3dCommandQueue, &swapDesc, &d3dSwapChain);

    // VTABLE ADRESSEN LESEN
    void** swapChainVTable = *(void***)d3dSwapChain;
    void** queueVTable = *(void***)d3dCommandQueue;

    void* presentAddr = swapChainVTable[8]; // Present
    void* executeAddr = queueVTable[10];    // ExecuteCommandLists (Index 10 = Offset 54 in x64?) - Prüfen wir
    // Bei D3D12CommandQueue ist ExecuteCommandLists der 10. Eintrag (0-basiert) in ID3D12CommandQueue
    // IUnknown(3) + ID3D12Pageable(0) + ID3D12DeviceChild(1) + ID3D12Object(2) ...
    // Der Prompt sagte 54 -> 54 / 8 Bytes = 6.75? Nein.
    // Index 10 ist korrekt für ExecuteCommandLists.

    MH_Initialize();

    if (MH_CreateHook(presentAddr, &hkPresent, (LPVOID*)&oPresent) != MH_OK) return false;
    if (MH_EnableHook(presentAddr) != MH_OK) return false;

    if (MH_CreateHook(executeAddr, &hkExecuteCommandLists, (LPVOID*)&oExecuteCommandLists) != MH_OK) return false;
    if (MH_EnableHook(executeAddr) != MH_OK) return false;

    // Cleanup
    d3dSwapChain->Release();
    d3dCommandQueue->Release();
    d3dDevice->Release();
    DestroyWindow(hWnd);

    return true;
}
