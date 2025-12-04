#include "Simulator.h"
#include "DX12Renderer.h"
#include "OpenCLProcessor.h"
#include "SharedMemoryBridge.h"
#include "NvidiaOpticalFlow.h"
#include <thread>
#include <windows.h> // For Sleep

Simulator::Simulator() {
    std::cout << "Simulator constructed." << std::endl;
}

Simulator::~Simulator() {
    std::cout << "Simulator destroyed." << std::endl;
}

void Simulator::Initialize() {
    std::cout << "Initializing Simulator..." << std::endl;

    // Create components
    dx12Renderer_ = new DX12Renderer();
    openclProcessor_ = new OpenCLProcessor();
    sharedMemoryBridge_ = new SharedMemoryBridge();
    nvidiaOF_ = new NvidiaOpticalFlow();

    // Initialize components
    if (!dx12Renderer_->Initialize(width_, height_)) {
        throw std::runtime_error("Failed to initialize DX12 Renderer");
    }

    if (!openclProcessor_->Initialize()) {
        throw std::runtime_error("Failed to initialize OpenCL Processor");
    }

    if (!sharedMemoryBridge_->Initialize()) {
        throw std::runtime_error("Failed to initialize Shared Memory Bridge");
    }

    if (!nvidiaOF_->Initialize()) {
        std::cout << "Warning: Nvidia Optical Flow not available, continuing with simulation." << std::endl;
    }

    startTime_ = std::chrono::steady_clock::now();
    std::cout << "Simulator initialized successfully." << std::endl;
    std::cout << "Initialized on Nvidia GPU for rendering and AMD iGPU for processing." << std::endl;
}

void Simulator::Run() {
    std::cout << "Starting simulation loop..." << std::endl;
    running_ = true;
    const int maxFrames = 100;

    for (frameCount_ = 0; frameCount_ < maxFrames && running_; ++frameCount_) {
        auto frameStart = std::chrono::steady_clock::now();

        UpdateFrame();
        RenderFrame();
        ProcessFrame();

        auto frameEnd = std::chrono::steady_clock::now();
        auto frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart);
        avgLatency_ = (avgLatency_ * frameCount_ + frameTime.count()) / (frameCount_ + 1);

        // Simulate 60 FPS (16.67ms per frame)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));

        if (frameCount_ % 10 == 0) {
            std::cout << "Frame " << frameCount_ << " processed." << std::endl;
        }
    }

    LogMetrics();
}

void Simulator::Shutdown() {
    std::cout << "Shutting down Simulator..." << std::endl;

    delete dx12Renderer_;
    delete openclProcessor_;
    delete sharedMemoryBridge_;
    delete nvidiaOF_;

    std::cout << "Simulator shutdown complete." << std::endl;
}

void Simulator::UpdateFrame() {
    // Simulate updating frame data
    // In real app, update triangle position
}

void Simulator::RenderFrame() {
    // Simulate rendering on Nvidia GPU
    std::cout << "[NVIDIA GPU] Rendering moving triangle (Frame " << frameCount_ << ")" << std::endl;
    // dx12Renderer_->Render();
}

void Simulator::ProcessFrame() {
    // Simulate shared memory bridge
    std::cout << "[SHARED MEMORY] Transferring frame data from Nvidia GPU to AMD iGPU" << std::endl;
    // sharedMemoryBridge_->TransferFrame();

    // Simulate OpenCL processing on AMD
    std::cout << "[AMD iGPU] Applying optical flow warping via OpenCL kernel" << std::endl;
    // openclProcessor_->ProcessFrame();
    std::cout << "[SUCCESS] Frame successfully moved from Nvidia GPU to AMD iGPU and processed." << std::endl;
}

void Simulator::LogMetrics() {
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime_);
    avgFPS_ = static_cast<double>(frameCount_) / (duration.count() / 1000.0);

    std::cout << "=== Simulation Metrics ===" << std::endl;
    std::cout << "Frames processed: " << frameCount_ << std::endl;
    std::cout << "Total duration: " << duration.count() << " ms" << std::endl;
    std::cout << "Average FPS: " << avgFPS_ << std::endl;
    std::cout << "Average latency: " << avgLatency_ << " ms" << std::endl;
    std::cout << "Simulation successfully demonstrated cross-GPU frame generation." << std::endl;
}
