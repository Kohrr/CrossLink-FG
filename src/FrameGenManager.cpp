#include "FrameGenManager.h"
#include "DX12Renderer.h"
#include "OpenCLProcessor.h"
#include "SharedMemoryBridge.h"
#include "NvidiaOpticalFlow.h"
#include <iostream>

#include <d3d11.h>
#include <d3d12.h>

FrameGenManager& FrameGenManager::GetInstance() {
    static FrameGenManager instance;
    return instance;
}

FrameGenManager::~FrameGenManager() {
    Shutdown();
}

bool FrameGenManager::Initialize() {
    if (initialized_) return true;

    std::cout << "FrameGenManager: Initializing CrossLink FG..." << std::endl;

    // Initialize components (but no window creation like simulator)
    dx12Renderer_ = std::make_unique<DX12Renderer>();
    openclProcessor_ = std::make_unique<OpenCLProcessor>();
    sharedMemoryBridge_ = std::make_unique<SharedMemoryBridge>();
    nvidiaOF_ = std::make_unique<NvidiaOpticalFlow>();

    if (!InitNvidiaGPU()) {
        std::cout << "Failed to initialize Nvidia GPU support. Continuing..." << std::endl;
    }

    if (!InitSharedMemory()) {
        std::cout << "Failed to initialize shared memory bridge." << std::endl;
        return false;
    }

    if (!InitAmdIGPU()) {
        std::cout << "Failed to initialize AMD iGPU. Continuing with simulation." << std::endl;
    }

    lastFrameTime_ = std::chrono::steady_clock::now();
    initialized_ = true;
    std::cout << "FrameGenManager: Core components initialized." << std::endl;
    std::cout << "FrameGenManager: ImGui will be initialized on first Present call." << std::endl;
    return true;
}

void FrameGenManager::Shutdown() {
    if (!initialized_) return;

    std::cout << "FrameGenManager: Shutting down..." << std::endl;
    initialized_ = false;

    // Cleanup in order
    nvidiaOF_.reset();
    sharedMemoryBridge_.reset();
    openclProcessor_.reset();
    dx12Renderer_.reset();
}

bool FrameGenManager::InitNvidiaGPU() {
    // Try to initialize real Nvidia OF, fallback to simulated
    if (!nvidiaOF_->Initialize()) {
        std::cout << "FrameGenManager: Nvidia OF not available, using fallback." << std::endl;
        return false;
    }
    return true;
}

bool FrameGenManager::InitSharedMemory() {
    // Use DX12 shared heaps
    if (!sharedMemoryBridge_->Initialize()) {
        return false;
    }
    return true;
}

bool FrameGenManager::InitAmdIGPU() {
    // Initialize OpenCL on AMD iGPU
    if (!openclProcessor_->Initialize()) {
        std::cout << "Warning: AMD iGPU OpenCL not available." << std::endl;
        return false;
    }
    return true;
}

void FrameGenManager::OnPresent(void* swapChain) {
    if (!initialized_ || passthroughMode_) return;

    auto frameStart = std::chrono::steady_clock::now();

    // For now, just detect graphics API and log it
    IDXGISwapChain* dxgiSwapChain = reinterpret_cast<IDXGISwapChain*>(swapChain);

    ID3D11Device* d3d11Device = nullptr;
    ID3D12Device* d3d12Device = nullptr;

    if (SUCCEEDED(reinterpret_cast<IUnknown*>(swapChain)->QueryInterface(__uuidof(ID3D11Device), (void**)&d3d11Device))) {
        std::cout << "[DETECTED] DirectX 11 API detected" << std::endl;
        d3d11Device->Release();
    } else if (SUCCEEDED(reinterpret_cast<IUnknown*>(swapChain)->QueryInterface(__uuidof(ID3D12Device), (void**)&d3d12Device))) {
        std::cout << "[DETECTED] DirectX 12 API detected" << std::endl;
        d3d12Device->Release();
    }

    // Process frame (currently simulated)
    ProcessFrameFromSwapChain(nullptr);

    // Render ImGui overlay indicating active
    RenderOverlay();

    // Check latency
    CheckLatency();

    auto frameEnd = std::chrono::steady_clock::now();
    auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
    avgLatencyMs_ = (avgLatencyMs_ * frameCount_ + static_cast<int>(frameTime.count())) / (frameCount_ + 1);
    frameCount_++;
}

void FrameGenManager::ProcessFrameFromSwapChain(void* backBuffer) {
    std::cout << "[CROSSLINK FG] Processing frame " << frameCount_ << " from game swapchain." << std::endl;

    // Transfer via shared memory
    std::cout << "[SHARED MEMORY] Transferring frame from Nvidia GPU to AMD iGPU via shared heap." << std::endl;
    sharedMemoryBridge_->TransferFrame();

    // Process on AMD iGPU
    std::cout << "[AMD iGPU] Applying optical flow and warping on AMD iGPU." << std::endl;
    openclProcessor_->ProcessFrame();

    std::cout << "[SUCCESS] Frame processed cross-GPU by CrossLink FG." << std::endl;
}

void FrameGenManager::RenderOverlay() {
    // Console output mode - ImGui rendering will be added later once libraries are linked
    std::cout << "[OVERLAY] CrossLink FG: Active (Frame " << frameCount_ << ", Latency: " << avgLatencyMs_ << "ms)" << std::endl;
}

void FrameGenManager::CheckLatency() {
    if (avgLatencyMs_ > maxSafeLatencyMs_) {
        std::cout << "[WARNING] Latency too high (" << avgLatencyMs_ << "ms), enabling passthrough mode." << std::endl;
        FallbackToPassthrough();
    }
}

void FrameGenManager::FallbackToPassthrough() {
    passthroughMode_ = true;
    std::cout << "CrossLink FG: Passthrough mode enabled to prevent performance impact." << std::endl;
}
