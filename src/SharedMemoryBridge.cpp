#include "SharedMemoryBridge.h"
#include <iostream>
#include <dxgi.h>       // For IDXGIResource
#include <d3d11.h>      // For ID3D11Device

SharedMemoryBridge::SharedMemoryBridge() {
    std::cout << "SharedMemoryBridge constructed." << std::endl;
}

SharedMemoryBridge::~SharedMemoryBridge() {
    Shutdown();
    std::cout << "SharedMemoryBridge destroyed." << std::endl;
}

bool SharedMemoryBridge::Initialize() {
    std::cout << "SharedMemoryBridge: Initializing DX11 shared resources..." << std::endl;
    // For now, initialization is just success - shared handle created as needed per frame
    std::cout << "SharedMemoryBridge: Ready for cross-GPU sharing." << std::endl;
    initialized_ = true;
    return true;
}

void SharedMemoryBridge::Shutdown() {
    if (sharedHandle_) {
        CloseHandle(sharedHandle_);
        sharedHandle_ = nullptr;
    }
    initialized_ = false;
    std::cout << "SharedMemoryBridge: Shutdown complete." << std::endl;
}

bool SharedMemoryBridge::CreateSharedHandle(ID3D11Texture2D* texture, ID3D11Device* device, HANDLE* outHandle) {
    if (!initialized_ || !texture || !device) {
        std::cout << "SharedMemoryBridge: Invalid parameters for shared handle creation." << std::endl;
        return false;
    }

    // First, check if the texture was created with D3D11_RESOURCE_MISC_SHARED flag
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    if (!(desc.MiscFlags & D3D11_RESOURCE_MISC_SHARED)) {
        std::cout << "SharedMemoryBridge: Texture not created with D3D11_RESOURCE_MISC_SHARED. Cannot create shared handle." << std::endl;
        return false;
    }

    // Query for IDXGIResource interface to create shared handle
    IDXGIResource* dxgiResource = nullptr;
    HRESULT hr = texture->QueryInterface(__uuidof(IDXGIResource), (void**)&dxgiResource);
    if (FAILED(hr)) {
        std::cout << "SharedMemoryBridge: Failed to query IDXGIResource interface. HRESULT: " << std::hex << hr << std::dec << std::endl;
        return false;
    }

    // Create shared handle
    HANDLE handle = nullptr;
    hr = dxgiResource->GetSharedHandle(&handle);

    // Release the IDXGIResource
    dxgiResource->Release();

    if (FAILED(hr) || !handle) {
        std::cout << "SharedMemoryBridge: Failed to create shared handle. HRESULT: " << std::hex << hr << std::dec << std::endl;
        return false;
    }

    // Store the handle and return it
    if (sharedHandle_) {
        CloseHandle(sharedHandle_); // Close previous handle
    }
    sharedHandle_ = handle;
    *outHandle = handle;

    std::cout << "SharedMemoryBridge: Successfully created shared handle for DX11 texture." << std::endl;
    return true;
}

void SharedMemoryBridge::TransferFrame() {
    if (!sharedHandle_) {
        std::cout << "SharedMemoryBridge: No shared handle available for transfer." << std::endl;
        return;
    }

    std::cout << "SharedMemoryBridge: Frame data available via shared handle " << sharedHandle_ << std::endl;
    std::cout << "SharedMemoryBridge: In production, this would be accessed by AMD iGPU via OpenCL/OpenGL interop" << std::endl;

    // In production code, we would:
    // 1. Pass sharedHandle_ to AMD GPU OpenCL context
    // 2. Use clCreateFromGLTexture or similar to import shared texture
    // 3. Process the frame data directly on AMD iGPU
}
