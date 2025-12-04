#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <chrono>
#include <iostream>

class DX12Renderer;
class OpenCLProcessor;
class SharedMemoryBridge;
class NvidiaOpticalFlow;

class Simulator {
public:
    Simulator();
    ~Simulator();

    void Initialize();
    void Run();
    void Shutdown();

private:
    void UpdateFrame();
    void RenderFrame();
    void ProcessFrame();
    void LogMetrics();

    // Windows
    void* nvidiaWindow_ = nullptr;
    void* amdWindow_ = nullptr;

    // Components
    DX12Renderer* dx12Renderer_ = nullptr;
    OpenCLProcessor* openclProcessor_ = nullptr;
    SharedMemoryBridge* sharedMemoryBridge_ = nullptr;
    NvidiaOpticalFlow* nvidiaOF_ = nullptr;

    // Metrics
    std::chrono::steady_clock::time_point startTime_;
    uint64_t frameCount_ = 0;
    double avgFPS_ = 0.0;
    double avgLatency_ = 0.0;

    // Simulation parameters
    const uint32_t width_ = 1920;
    const uint32_t height_ = 1080;

    bool running_ = false;
};

#endif // SIMULATOR_H
