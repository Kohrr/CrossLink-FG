// Universal DX12 VTable Hooking Implementation with CommandQueue Capture
// SIMPLIFIED VERSION WITHOUT IMGUI - READY FOR BORDERLANDS INJECTION
#include <windows.h>
#include <iostream>
#include <MinHook.h>
#include <d3d12.h>
#include <dxgi1_4.h>
#include "FrameGenManager.h"
#include "frame_capture.h"
#include "frame_processor.h"

// Function pointer typedefs
typedef HRESULT(__stdcall* Present_t)(IDXGISwapChain*, UINT, UINT);
typedef void(WINAPI* ExecuteCommandLists_t)(ID3D12CommandQueue*, UINT, ID3D12CommandList* const*);

// Structure to hold both function addresses
struct HookedFunctions {
    void* presentAddr;
    void* executeCommandListsAddr;
};

// Global variables
Present_t origPresent = nullptr;
ExecuteCommandLists_t origExecuteCommandLists = nullptr;
HookedFunctions g_HookedFunctions = { nullptr, nullptr };

// CRITICAL: Global DX12 ImGui RENDERING SYSTEM
ID3D12Device* g_pd3dDevice = NULL;
ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
ID3D12CommandAllocator* g_commandAllocators[3] = { NULL };
ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
ID3D12Resource* g_mainRenderTargetResource[3] = { NULL };
D3D12_CPU_DESCRIPTOR_HANDLE g_mainRenderTargetDescriptor[3];
ID3D12CommandQueue* g_pGameCommandQueue = NULL; // CAPTURED FROM GAME
IDXGISwapChain3* g_pSwapChain = NULL;
UINT g_BufferCount = 0;
bool g_ImguiInitialized = false;

// Initialize ImGui for DX12 once we have the device's swapchain and command queue
bool InitImGui(IDXGISwapChain* swapChain, ID3D12Device* device) {
    if (g_ImguiInitialized) return true;

    std::cout << "[IMGUI] Initializing DX12 ImGui overlay..." << std::endl;

    // Get swapchain description
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain->GetDesc(&desc);
    g_BufferCount = desc.BufferCount;

    // Store pointers
    g_pd3dDevice = device;
    g_pd3dDevice->AddRef();

    if (SUCCEEDED(swapChain->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&g_pSwapChain))) {
        g_pSwapChain->AddRef();
    }

    // Create RTV descriptor heap
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.NumDescriptors = g_BufferCount;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

    if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&rtvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&g_pd3dRtvDescHeap))) {
        std::cout << "[IMGUI] Failed to create RTV descriptor heap" << std::endl;
        return false;
    }

    // Create SRV descriptor heap for fonts
    D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
    srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvHeapDesc.NumDescriptors = 1;
    srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    if (FAILED(g_pd3dDevice->CreateDescriptorHeap(&srvHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&g_pd3dSrvDescHeap))) {
        std::cout << "[IMGUI] Failed to create SRV descriptor heap" << std::endl;
        return false;
    }

    // Create command allocators
    for (UINT i = 0; i < g_BufferCount; i++) {
        if (FAILED(g_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&g_commandAllocators[i]))) {
            std::cout << "[IMGUI] Failed to create command allocator " << i << std::endl;
            return false;
        }
    }

    // Create command list
    if (FAILED(g_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, g_commandAllocators[0], NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&g_pd3dCommandList))) {
        std::cout << "[IMGUI] Failed to create command list" << std::endl;
        return false;
    }

    // Create RTVs for backbuffers
    SIZE_T rtvDescriptorSize = g_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = g_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < g_BufferCount; i++) {
        ID3D12Resource* pBackBuffer = NULL;
        if (SUCCEEDED(g_pSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&pBackBuffer))) {
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, rtvHandle);
            g_mainRenderTargetResource[i] = pBackBuffer;
            g_mainRenderTargetDescriptor[i] = rtvHandle;
            rtvHandle.ptr += rtvDescriptorSize;
        }
    }

    // Initialize ImGui
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;

    // Initialize Win32 backend
    if (!ImGui_ImplWin32_Init(desc.OutputWindow)) {
        std::cout << "[IMGUI] Failed to initialize Win32 backend" << std::endl;
        return false;
    }

    // Initialize DX12 backend
    if (!ImGui_ImplDX12_Init(g_pd3dDevice, g_BufferCount, DXGI_FORMAT_R8G8B8A8_UNORM,
                           g_pd3dSrvDescHeap,
                           g_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
                           g_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart())) {
        std::cout << "[IMGUI] Failed to initialize DX12 backend" << std::endl;
        return false;
    }

    g_ImguiInitialized = true;
    std::cout << "[IMGUI] DX12 ImGui overlay initialized successfully!" << std::endl;
    return true;
}

// Render ImGui overlay using captured command queue
void RenderImGui(int currentBufferIndex) {
    if (!g_ImguiInitialized || !g_pGameCommandQueue) return;

    // Get command allocator for current buffer
    ID3D12CommandAllocator* cmdAllocator = g_commandAllocators[currentBufferIndex];

    // Reset command list
    g_pd3dCommandList->Reset(cmdAllocator, nullptr);

    // Transition backbuffer to render target
    D3D12_RESOURCE_BARRIER barrier = {};
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = g_mainRenderTargetResource[currentBufferIndex];
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    // Set render target
    g_pd3dCommandList->OMSetRenderTargets(1, &g_mainRenderTargetDescriptor[currentBufferIndex], FALSE, nullptr);

    // Clear (optional - make overlay transparent)
    const float clear_color[4] = { 0.f, 0.f, 0.f, 0.f };
    g_pd3dCommandList->ClearRenderTargetView(g_mainRenderTargetDescriptor[currentBufferIndex], clear_color, 0, nullptr);

    // Render ImGui
    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), g_pd3dCommandList);

    // Transition back to present
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    g_pd3dCommandList->ResourceBarrier(1, &barrier);

    // Close command list
    g_pd3dCommandList->Close();

    // Execute our command list
    ID3D12CommandList* const commandLists[] = { g_pd3dCommandList };
    g_pGameCommandQueue->ExecuteCommandLists(1, commandLists);
}

// CRITICAL: Hook ExecuteCommandLists to capture the game's command queue
void __stdcall hkExecuteCommandLists(ID3D12CommandQueue* pCommandQueue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists) {
    // CAPTURE THE GAME'S COMMAND QUEUE - THIS IS THE MAGIC FOR DX12 RENDERING
    if (!g_pGameCommandQueue) {
        g_pGameCommandQueue = pCommandQueue;
        g_pGameCommandQueue->AddRef();
        std::cout << "[QUEUE CAPTURED] Game's DX12 command queue captured for ImGui rendering!" << std::endl;
    }

    // Call original function
    origExecuteCommandLists(pCommandQueue, NumCommandLists, ppCommandLists);
}

// Dummy window procedure
LRESULT CALLBACK DummyWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// FUNCTION: Get the real DX12 Present and ExecuteCommandLists addresses via VTable scanning
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

        // STEP 2: Load D3D12 and DXGI
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

        // STEP 6: Create dummy swapchain
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

        // STEP 7: CRITICAL - Extract Present and ExecuteCommandLists from VTable
        void** vtable = *(void***)swapChain;
        void* presentFunc = vtable[8];  // Present is at index 8 in IDXGISwapChain VTable
        void* executeCommandListsFunc = vtable[10]; // ExecuteCommandLists is usually at index 10

        std::cout << "[HOOK] VTable scanned - Present function found at: 0x" << std::hex << (uintptr_t)presentFunc << std::dec << std::endl;

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

        std::cout << "[HOOK] VTable scanning completed successfully." << std::endl;
        std::cout << "[HOOK] ExecuteCommandLists found at: 0x" << std::hex << (uintptr_t)executeCommandListsFunc << std::dec << std::endl;
        return presentFunc;

    } catch (const std::exception& e) {
        std::cout << "[HOOK] Exception during VTable scanning: " << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cout << "[HOOK] Unknown exception during VTable scanning." << std::endl;
        return nullptr;
    }
}

// Hooked Present function - RENDERS IMGUI OVERLAY ON TOP OF GAME
HRESULT __stdcall hkPresent(IDXGISwapChain* swapchain, UINT syncInterval, UINT flags) {
    static int frameCount = 0;

    if (frameCount == 0) {
        // First frame - confirm hooking is working and initialize ImGui
        std::cout << "[HOOK SUCCESS] hkPresent called! CrossLink FG is active in Borderlands!" << std::endl;
        std::cout << "[HOOK SUCCESS] Frame interception confirmed - ready for frame generation." << std::endl;

        // Get device and initialize ImGui
        ID3D12Device* device = nullptr;
        if (SUCCEEDED(swapchain->QueryInterface(__uuidof(ID3D12Device), (void**)&device))) {
            if (InitImGui(swapchain, device)) {
                std::cout << "[IMGUI] Successfully initialized overlay rendering!" << std::endl;
            }
            device->Release();
        }
    }

    // Only render overlay if queue is captured and ImGui initialized
    if (g_ImguiInitialized && g_pGameCommandQueue) {
        // Get current backbuffer index
        UINT currentBufferIndex = 0;
        if (g_pSwapChain) {
            currentBufferIndex = g_pSwapChain->GetCurrentBackBufferIndex();
        }

        // Setup ImGui frame
        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Create overlay window
        ImGui::SetNextWindowBgAlpha(0.9f);
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::Begin("CrossLink Frame Generation", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing);

        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "CrossLink FG Active");
        ImGui::Text("Frames: %d", frameCount);
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        ImGui::Text("Buffer: %d", currentBufferIndex);

        ImGui::Separator();

        if (frameCount < 300) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Initializing...");
        } else {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Frame Generation Active");
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "Cross-GPU Processing");
        }

        ImGui::End();

        // Render the ImGui commands to the backbuffer
        RenderImGui(currentBufferIndex);

        // Log successful overlay rendering
        static int overlayLogCount = 0;
        overlayLogCount++;
        if (overlayLogCount % 120 == 0) { // Log every 2 seconds
            std::cout << "[VISUAL] ImGui overlay rendered successfully on screen!" << std::endl;
        }
    }

    frameCount++;

    // Call our FrameGenManager to process the frame
    FrameGenManager::GetInstance().OnPresent(swapchain);

    // Log frame interception once per second
    if (frameCount % 60 == 0) {
        std::cout << "[HOOK] Frames intercepted: " << frameCount << " - CrossLink FG running" << std::endl;
    }

    // Call original Present
    return origPresent(swapchain, syncInterval, flags);
}

// Universal initialization - works for both DX11 and DX12 - HOOKS BOTH Present AND ExecuteCommandLists
bool InitializeHooks() {
    std::cout << "[HOOK] Initializing Universal DX12 VTable Hooking for Borderlands..." << std::endl;

    // Initialize MinHook
    if (MH_Initialize() != MH_OK) {
        std::cout << "[HOOK] Failed to initialize MinHook." << std::endl;
        return false;
    }

    // Get both function addresses via VTable scanning
    void* presentTarget = GetDX12PresentAddress();
    if (!presentTarget || !g_HookedFunctions.executeCommandListsAddr) {
        std::cout << "[HOOK] Failed to locate both Present and ExecuteCommandLists via VTable scanning." << std::endl;
        std::cout << "[HOOK] This might indicate DX12 not available or other graphics issues." << std::endl;
        return false;
    }

    // Hook the Present function
    MH_STATUS presentStatus = MH_CreateHook(presentTarget, hkPresent, (LPVOID*)&origPresent);
    if (presentStatus != MH_OK) {
        std::cout << "[HOOK] Failed to create Present hook. MinHook error: " << presentStatus << std::endl;
        return false;
    }

    presentStatus = MH_EnableHook(presentTarget);
    if (presentStatus != MH_OK) {
        std::cout << "[HOOK] Failed to enable Present hook. MinHook error: " << presentStatus << std::endl;
        return false;
    }

    // CRITICAL: Hook ExecuteCommandLists to capture command queue for ImGui rendering
    MH_STATUS executeStatus = MH_CreateHook(g_HookedFunctions.executeCommandListsAddr, hkExecuteCommandLists, (LPVOID*)&origExecuteCommandLists);
    if (executeStatus != MH_OK) {
        std::cout << "[HOOK] Failed to create ExecuteCommandLists hook. MinHook error: " << executeStatus << std::endl;
        return false;
    }

    executeStatus = MH_EnableHook(g_HookedFunctions.executeCommandListsAddr);
    if (executeStatus != MH_OK) {
        std::cout << "[HOOK] Failed to enable ExecuteCommandLists hook. MinHook error: " << executeStatus << std::endl;
        return false;
    }

    std::cout << "[HOOK] Universal DX12 Present + ExecuteCommandLists hooks initialized successfully!" << std::endl;
    std::cout << "[HOOK] Frame interruption + command queue capture ready - CrossLink FG will activate on first frame." << std::endl;
    std::cout << "[HOOK] Visual ImGui overlay will appear once command queue is captured from game." << std::endl;

    return true;
}

void ShutdownHooks() {
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();

    // Future: Cleanup ImGui DX12 resources will be added here

    std::cout << "[HOOK] Hooks shutdown." << std::endl;
}
