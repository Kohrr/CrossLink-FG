#include "frame_capture.h"
#include <iostream>
#include <d3d12.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

// Static members
bool FrameCapture::s_initialized = false;
ID3D12Device* FrameCapture::s_device = nullptr;
ID3D12CommandQueue* FrameCapture::s_cmdQueue = nullptr;
ID3D12CommandAllocator* FrameCapture::s_cmdAllocator = nullptr;
ID3D12GraphicsCommandList* FrameCapture::s_cmdList = nullptr;
ID3D12Resource* FrameCapture::s_readbackBuffer = nullptr;
UINT64 FrameCapture::s_frameCounter = 0;
CapturedFrame FrameCapture::s_latestFrame = {};

bool FrameCapture::Initialize(ID3D12Device* device) {
    if (s_initialized) return true;

    std::cout << "[FRAME CAPTURE] Initializing..." << std::endl;
    s_device = device;
    s_device->AddRef();

    // Command Queue erstellen
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    if (FAILED(s_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&s_cmdQueue)))) {
        std::cerr << "[FRAME CAPTURE ERROR] Failed to create command queue!" << std::endl;
        return false;
    }

    // Command Allocator
    if (FAILED(s_device->CreateCommandAllocator(
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        IID_PPV_ARGS(&s_cmdAllocator)))) {
        std::cerr << "[FRAME CAPTURE ERROR] Failed to create command allocator!" << std::endl;
        return false;
    }

    // Command List
    if (FAILED(s_device->CreateCommandList(
        0,
        D3D12_COMMAND_LIST_TYPE_DIRECT,
        s_cmdAllocator,
        nullptr,
        IID_PPV_ARGS(&s_cmdList)))) {
        std::cerr << "[FRAME CAPTURE ERROR] Failed to create command list!" << std::endl;
        return false;
    }

    s_cmdList->Close();
    s_initialized = true;

    std::cout << "[FRAME CAPTURE SUCCESS] Initialization complete" << std::endl;
    return true;
}

void FrameCapture::Shutdown() {
    if (!s_initialized) return;

    std::cout << "[FRAME CAPTURE] Shutting down..." << std::endl;

    if (s_cmdList) s_cmdList->Release();
    if (s_cmdAllocator) s_cmdAllocator->Release();
    if (s_cmdQueue) s_cmdQueue->Release();
    if (s_readbackBuffer) s_readbackBuffer->Release();
    if (s_device) s_device->Release();

    s_initialized = false;
    std::cout << "[FRAME CAPTURE] Shutdown complete" << std::endl;
}

bool FrameCapture::CaptureFromSwapChain(IDXGISwapChain* swapChain) {
    if (!s_initialized || !swapChain) return false;

    // 1. Backbuffer Texture holen
    ComPtr<ID3D12Resource> backBuffer;
    if (FAILED(swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
        std::cerr << "[FRAME CAPTURE ERROR] Failed to get back buffer!" << std::endl;
        return false;
    }

    // 2. Texture Informationen
    D3D12_RESOURCE_DESC desc = backBuffer->GetDesc();

    // 3. Readback Buffer erstellen (falls nötig)
    if (!s_readbackBuffer ||
        s_readbackBuffer->GetDesc().Width != desc.Width ||
        s_readbackBuffer->GetDesc().Height != desc.Height) {

        if (s_readbackBuffer) {
            s_readbackBuffer->Release();
            s_readbackBuffer = nullptr;
        }

        if (!CreateReadbackBuffer(desc.Width, desc.Height, desc.Format)) {
            return false;
        }
    }

    // 4. Frame zählen
    s_frameCounter++;

    // 5. Frame-Daten speichern
    s_latestFrame.width = static_cast<UINT>(desc.Width);
    s_latestFrame.height = static_cast<UINT>(desc.Height);
    s_latestFrame.format = desc.Format;
    s_latestFrame.frameNumber = s_frameCounter;
    QueryPerformanceCounter(&s_latestFrame.timestamp);

    // 6. Texture kopieren (asynchron - würde hier implementiert werden)
    // CopyTextureToReadback(backBuffer.Get());

    // Debug-Ausgabe (jeden 60. Frame)
    if (s_frameCounter % 60 == 0) {
        std::cout << "[FRAME CAPTURE] Captured frame #" << s_frameCounter
                  << " (" << s_latestFrame.width << "x" << s_latestFrame.height << ")" << std::endl;
    }

    return true;
}

const CapturedFrame* FrameCapture::GetLatestFrame() {
    return &s_latestFrame;
}

bool FrameCapture::CreateReadbackBuffer(UINT width, UINT height, DXGI_FORMAT format) {
    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_READBACK;
    heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC resourceDesc = {};
    resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDesc.Alignment = 0;
    resourceDesc.Width = width * height * 4; // RGBA8
    resourceDesc.Height = 1;
    resourceDesc.DepthOrArraySize = 1;
    resourceDesc.MipLevels = 1;
    resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    resourceDesc.SampleDesc.Count = 1;
    resourceDesc.SampleDesc.Quality = 0;
    resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

    if (FAILED(s_device->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &resourceDesc,
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&s_readbackBuffer)))) {

        std::cerr << "[FRAME CAPTURE ERROR] Failed to create readback buffer!" << std::endl;
        return false;
    }

    return true;
}

void FrameCapture::CopyTextureToReadback(ID3D12Resource* sourceTexture) {
    // Diese Funktion würde den eigentlichen Texture-Copy implementieren
    // Komplex wegen Resource Barriers, Command Lists, etc.
    // Für MVP erstmal weglassen
}
