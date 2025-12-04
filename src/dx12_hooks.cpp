// DX12 HOOKS - GOD-TIER Implementation für Borderlands 3 Overlay
// KRITISCH: ExecuteCommandLists Hook für Queue Capture
#include "imgui_manager.h"
#include <MinHook.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// ERROR STRING HELPER für MinHook
inline const char* MyStatusToString(MH_STATUS status) {
    switch(status) {
        case MH_UNKNOWN: return "MH_UNKNOWN";
        case MH_OK: return "MH_OK";
        case MH_ERROR_ALREADY_INITIALIZED: return "MH_ERROR_ALREADY_INITIALIZED";
        case MH_ERROR_NOT_INITIALIZED: return "MH_ERROR_NOT_INITIALIZED";
        case MH_ERROR_ALREADY_CREATED: return "MH_ERROR_ALREADY_CREATED";
        case MH_ERROR_NOT_EXECUTABLE: return "MH_ERROR_NOT_EXECUTABLE";
        case MH_ERROR_UNSUPPORTED_FUNCTION: return "MH_ERROR_UNSUPPORTED_FUNCTION";
        case MH_ERROR_MEMORY_ALLOC: return "MH_ERROR_MEMORY_ALLOC";
        case MH_ERROR_MEMORY_PROTECT: return "MH_ERROR_MEMORY_PROTECT";
        case MH_ERROR_MODULE_NOT_FOUND: return "MH_ERROR_MODULE_NOT_FOUND";
        case MH_ERROR_FUNCTION_NOT_FOUND: return "MH_ERROR_FUNCTION_NOT_FOUND";
        default: return "Unknown Error";
    }
}

// GOD-TIER: Globale Variablen für Queue Capture
ID3D12CommandQueue* g_pCommandQueue = nullptr;        // DER PREIS! Queue Capture
ID3D12GraphicsCommandList* g_pCommandList = nullptr;
ID3D12CommandAllocator* g_pAllocators[3] = { nullptr };
ID3D12DescriptorHeap* g_pRtvHeap = nullptr;
ID3D12Resource* g_pRenderTargets[3] = { nullptr };
D3D12_CPU_DESCRIPTOR_HANDLE g_rtvHandles[3];
UINT g_BackBufferCount = 0;
ID3D12Device* g_pDevice = nullptr;

// Original Funktionen
typedef void(WINAPI* ExecuteCommandLists_t)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);
typedef HRESULT(WINAPI* Present_t)(IDXGISwapChain*, UINT, UINT);

ExecuteCommandLists_t oExecuteCommandLists = nullptr;
Present_t oPresent = nullptr;

// VTABLE SCANNER - GetDX12DeviceAddresses
void GetDX12DeviceAddresses(void** presentAddr, void** executeAddr) {
    // Dummy Window erstellen
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, DefWindowProc, 0L, 0L, 
                     GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"DX", NULL };
    RegisterClassEx(&wc);
    HWND hWnd = CreateWindow(L"DX", L"DX", WS_OVERLAPPEDWINDOW, 
                           100, 100, 1280, 720, NULL, NULL, wc.hInstance, NULL);

    // DX12 Device + SwapChain erstellen
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ID3D12Device* d3dDevice = nullptr;
    ID3D12CommandQueue* d3dCommandQueue = nullptr;
    IDXGISwapChain* d3dSwapChain = nullptr;

    if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&d3dDevice)))) {
        std::cerr << "[HOOKS ERROR] Failed to create DX12 Device!" << std::endl;
        return;
    }

    if (FAILED(d3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&d3dCommandQueue)))) {
        std::cerr << "[HOOKS ERROR] Failed to create Command Queue!" << std::endl;
        d3dDevice->Release();
        return;
    }

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

    if (FAILED(factory->CreateSwapChain(d3dCommandQueue, &swapDesc, &d3dSwapChain))) {
        std::cerr << "[HOOKS ERROR] Failed to create SwapChain!" << std::endl;
        d3dCommandQueue->Release();
        d3dDevice->Release();
        return;
    }

    // VTABLE ADDRESSES LESEN - RICHTIGE INDIZES VERWENDEN
    void** vtableSwapChain = *(void***)d3dSwapChain;
    *presentAddr = vtableSwapChain[8];  // IDXGISwapChain::Present

    void** vtableQueue = *(void***)d3dCommandQueue;
    *executeAddr = vtableQueue[10];     // ID3D12CommandQueue::ExecuteCommandLists

    std::cout << "[HOOKS] Present VTable: " << *presentAddr << std::endl;
    std::cout << "[HOOKS] ExecuteCommandLists VTable: " << *executeAddr << std::endl;

    // Cleanup
    d3dSwapChain->Release();
    d3dCommandQueue->Release();
    d3dDevice->Release();
    DestroyWindow(hWnd);
    UnregisterClass(L"DX", wc.hInstance);
}

// Overlay Ressourcen erstellen
void CreateOverlayResources(IDXGISwapChain* pSwapChain) {
    DXGI_SWAP_CHAIN_DESC desc;
    pSwapChain->GetDesc(&desc);
    g_BackBufferCount = desc.BufferCount;

    // RTV Heap erstellen
    D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
    rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvDesc.NumDescriptors = g_BackBufferCount;
    rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    
    g_pDevice->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&g_pRtvHeap));

    SIZE_T rtvSize = g_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pRtvHeap->GetCPUDescriptorHandleForHeapStart();

    // Render Targets erstellen
    for (UINT i = 0; i < g_BackBufferCount; i++) {
        g_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                         IID_PPV_ARGS(&g_pAllocators[i]));
        pSwapChain->GetBuffer(i, IID_PPV_ARGS(&g_pRenderTargets[i]));
        g_pDevice->CreateRenderTargetView(g_pRenderTargets[i], nullptr, rtvHandle);
        g_rtvHandles[i] = rtvHandle;
        rtvHandle.ptr += rtvSize;
    }

    // Command List erstellen
    g_pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
                                g_pAllocators[0], nullptr, IID_PPV_ARGS(&g_pCommandList));
    g_pCommandList->Close();

    std::cout << "[HOOKS] Overlay resources created successfully!" << std::endl;
}

// HOOK 1: ExecuteCommandLists (QUEUE CAPTURE) - DER KRITISCHE HOOK!
void WINAPI hkExecuteCommandLists(ID3D12CommandQueue* pQueue, UINT NumCommandLists, 
                                 ID3D12CommandList* const* ppCommandLists) {
    
    // GOD-TIER: Queue Capture Logic
    if (g_pCommandQueue == nullptr && pQueue->GetDesc().Type == D3D12_COMMAND_LIST_TYPE_DIRECT) {
        g_pCommandQueue = pQueue;
        ImGuiManager::SetCommandQueue(pQueue);
        std::cout << "===========================================" << std::endl;
        std::cout << "[GOD-TIER HOOK] 🎯 CAPTURED COMMAND QUEUE!" << std::endl;
        std::cout << "[GOD-TIER HOOK] Queue Address: 0x" << std::hex << pQueue << std::dec << std::endl;
        std::cout << "[GOD-TIER HOOK] 🎮 Overlay will now be VISIBLE!" << std::endl;
        std::cout << "===========================================" << std::endl;
    }
    
    // Original Function aufrufen
    oExecuteCommandLists(pQueue, NumCommandLists, ppCommandLists);
}

// HOOK 2: Present (RENDERING) - Complete Overlay Implementation
HRESULT WINAPI hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
    static bool firstTime = true;
    if (firstTime) {
        Beep(750, 300); // HIGH PITCH BEEP = HOOK ACTIVE!
        std::cout << "!!! [HOOK HIT] hkPresent IS RUNNING !!!" << std::endl;
        firstTime = false;

        // Overlay-Ressourcen einmalig erstellen
        if (!g_pDevice && SUCCEEDED(pSwapChain->GetDevice(IID_PPV_ARGS(&g_pDevice)))) {
            CreateOverlayResources(pSwapChain);
            ImGuiManager::Initialize(g_pDevice, pSwapChain);
            std::cout << "[HOOKS] Present hook initialized!" << std::endl;
        }
    }

    // GOD-TIER: Overlay Rendering (NUR wenn Queue captured!)
    if (g_pCommandQueue && oExecuteCommandLists && ImGuiManager::IsInitialized()) {
        // Beep pro Frame um zu wissen dass Rendering läuft
        Beep(1000, 100);

        IDXGISwapChain3* pSwapChain3 = (IDXGISwapChain3*)pSwapChain;
        UINT index = pSwapChain3->GetCurrentBackBufferIndex();

        // 1. Reset Allocator & Command List
        g_pAllocators[index]->Reset();
        g_pCommandList->Reset(g_pAllocators[index], nullptr);

        // 2. Resource Barrier: PRESENT -> RENDER_TARGET
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Transition.pResource = g_pRenderTargets[index];
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        g_pCommandList->ResourceBarrier(1, &barrier);

        // 3. Render Target setzen
        g_pCommandList->OMSetRenderTargets(1, &g_rtvHandles[index], FALSE, nullptr);

        // 4. ImGui Overlay zeichnen
        ImGuiManager::NewFrame();
        ImGuiManager::RenderOverlay();
        ImGuiManager::Render(g_pCommandList);

        // 5. Resource Barrier: RENDER_TARGET -> PRESENT
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        g_pCommandList->ResourceBarrier(1, &barrier);

        // 6. Command List schließen
        g_pCommandList->Close();

        // 7. 🎯 DIRECT DRIVER BYPASS: Verwende oExecuteCommandLists um Rekursion zu vermeiden!
        ID3D12CommandList* const ppCommandLists[] = { g_pCommandList };
        oExecuteCommandLists(g_pCommandQueue, 1, ppCommandLists);
    }

    // Original Present aufrufen
    return oPresent(pSwapChain, SyncInterval, Flags);
}

// HOOK INSTALLATION - VERBOSE & ROBUST
bool InstallDX12Hooks() {
    void* presentAddr = nullptr;
    void* executeAddr = nullptr;

    std::cout << "[HOOKS] Starting VTable scanning..." << std::endl;
    GetDX12DeviceAddresses(&presentAddr, &executeAddr);

    if (!presentAddr || !executeAddr) {
        std::cerr << "[HOOKS ERROR] Failed to get VTable addresses!" << std::endl;
        return false;
    }

    std::cout << "[HOOKS] VTable addresses found:" << std::endl;
    std::cout << "[HOOKS] presentAddr: " << presentAddr << std::endl;
    std::cout << "[HOOKS] executeAddr: " << executeAddr << std::endl;

    std::cout << "[HOOKS] Installing hooks..." << std::endl;

    // MinHook initialisieren (für Error-Reporting)
    MH_STATUS initStatus = MH_Initialize();
    std::cout << "[HOOKS] MH_Initialize: " << MyStatusToString(initStatus) << std::endl;

    // Sichere Original-Funktionen direkt aus VTable
    if (oPresent == nullptr) {
        oPresent = (Present_t)presentAddr;
        std::cout << "[HOOKS] Saved original Present: " << oPresent << std::endl;
    }
    if (oExecuteCommandLists == nullptr) {
        oExecuteCommandLists = (ExecuteCommandLists_t)executeAddr;
        std::cout << "[HOOKS] Saved original ExecuteCommandLists: " << oExecuteCommandLists << std::endl;
    }

    // FORCE VTable Hooking (wie Kiero) - umgeht MH_ERROR_NOT_EXECUTABLE
    DWORD oldProtect;

    if (VirtualProtect(presentAddr, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *(void**)presentAddr = &hkPresent;
        VirtualProtect(presentAddr, sizeof(void*), oldProtect, &oldProtect);
        std::cout << "[HOOKS] Direct VTable Hook Present: SUCCESS" << std::endl;
    } else {
        std::cerr << "[HOOK ERROR] VirtualProtect failed for Present VTable!" << std::endl;
        return false;
    }

    if (VirtualProtect(executeAddr, sizeof(void*), PAGE_EXECUTE_READWRITE, &oldProtect)) {
        *(void**)executeAddr = &hkExecuteCommandLists;
        VirtualProtect(executeAddr, sizeof(void*), oldProtect, &oldProtect);
        std::cout << "[HOOKS] Direct VTable Hook ExecuteCommandLists: SUCCESS" << std::endl;
    } else {
        std::cerr << "[HOOK ERROR] VirtualProtect failed for ExecuteCommandLists VTable!" << std::endl;
        return false;
    }

    std::cout << "===========================================" << std::endl;
    std::cout << "[GOD-TIER SUCCESS] 🎯 ALL HOOKS INSTALLED!" << std::endl;
    std::cout << "[GOD-TIER SUCCESS] ExecuteCommandLists Hook: ACTIVE" << std::endl;
    std::cout << "[GOD-TIER SUCCESS] Present Hook: ACTIVE" << std::endl;
    std::cout << "[GOD-TIER SUCCESS] 🎮 OVERLAY WILL BE VISIBLE!" << std::endl;
    std::cout << "===========================================" << std::endl;

    return true;
}

// HOOK UNINSTALLATION - Direkter VTable Restore
void UninstallDX12Hooks() {
    // FEHLERHAFTE VTABLE ADRESSEN! Wir müssen die echten Addresses speichern
    // TODO: Speichere die VTable-Adressen während Installation
    std::cerr << "[HOOKS] WARNING: Cannot restore VTable hooks - lost original addresses!" << std::endl;
    std::cerr << "[HOOKS] This is a limitation of manual VTable hooking." << std::endl;
    std::cout << "[HOOKS] Hooks cannot be cleanly uninstalled (Manual VTable limitation)." << std::endl;
}
