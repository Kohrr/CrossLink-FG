#ifndef FRAMEGENMANAGER_H
#define FRAMEGENMANAGER_H

#include <chrono>
#include <memory>
#include <cstdint>

// Forward declarations for ImGui/DX12
struct ID3D12Device;
struct ID3D12DescriptorHeap;
struct ID3D12GraphicsCommandList;
struct IDXGISwapChain;

// Forward declarations
struct ID3D11Device;
struct ID3D11DeviceContext;

class DX12Renderer;
class OpenCLProcessor;
class SharedMemoryBridge;
class NvidiaOpticalFlow;

class FrameGenManager {
public:
    static FrameGenManager& GetInstance();

    bool Initialize();
    void Shutdown();

    // Hook entry point for Present
    void OnPresent(void* swapChain);

    // Passthrough mode if latency too high
    void EnablePassthrough(bool enable) { passthroughMode_ = enable; }

private:
    FrameGenManager() = default;
    ~FrameGenManager();

    // Initialize components
    bool InitNvidiaGPU();
    bool InitSharedMemory();
    bool InitAmdIGPU();

    // Frame processing
    void ProcessFrameFromSwapChain(void* backBuffer);

    // Overlay rendering
    void RenderOverlay();

    // Metrics and safety
    void CheckLatency();
    void FallbackToPassthrough();

    // Components
    std::unique_ptr<DX12Renderer> dx12Renderer_;
    std::unique_ptr<OpenCLProcessor> openclProcessor_;
    std::unique_ptr<SharedMemoryBridge> sharedMemoryBridge_;
    std::unique_ptr<NvidiaOpticalFlow> nvidiaOF_;

    // State
    bool initialized_ = false;
    bool passthroughMode_ = false;
    int frameCount_ = 0;

    // Latency tracking
    std::chrono::steady_clock::time_point lastFrameTime_;
    int avgLatencyMs_ = 0;
    const int maxSafeLatencyMs_ = 16; // 16ms for 60 FPS

    // Game swapchain info
    void* gameSwapChain_ = nullptr;

    // ImGui DX12 state
    bool imguiInitialized_ = false;
    ID3D12Device* device_ = nullptr;
    ID3D12DescriptorHeap* srvDescHeap_ = nullptr;
    ID3D12DescriptorHeap* rtvDescHeap_ = nullptr;
    ID3D12GraphicsCommandList* commandList_ = nullptr;
    int bufferCount_ = 0;
};

#endif // FRAMEGENMANAGER_H
