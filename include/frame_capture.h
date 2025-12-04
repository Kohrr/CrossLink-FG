#ifndef FRAME_CAPTURE_H
#define FRAME_CAPTURE_H

#include <Windows.h>
#include <d3d12.h>
#include <dxgi.h>
#include <vector>
#include <cstdint>

struct CapturedFrame {
    std::vector<uint8_t> data;
    UINT width;
    UINT height;
    DXGI_FORMAT format;
    UINT64 frameNumber;
    LARGE_INTEGER timestamp;
};

class FrameCapture {
public:
    static bool Initialize(ID3D12Device* device);
    static void Shutdown();
    static bool CaptureFromSwapChain(IDXGISwapChain* swapChain);
    static const CapturedFrame* GetLatestFrame();
    static bool IsInitialized() { return s_initialized; }

private:
    static bool s_initialized;
    static ID3D12Device* s_device;
    static ID3D12CommandQueue* s_cmdQueue;
    static ID3D12CommandAllocator* s_cmdAllocator;
    static ID3D12GraphicsCommandList* s_cmdList;
    static ID3D12Resource* s_readbackBuffer;
    static UINT64 s_frameCounter;
    static CapturedFrame s_latestFrame;

    static bool CreateReadbackBuffer(UINT width, UINT height, DXGI_FORMAT format);
    static void CopyTextureToReadback(ID3D12Resource* sourceTexture);
};

#endif // FRAME_CAPTURE_H
