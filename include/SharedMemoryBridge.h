#ifndef SHAREDMEMORYBRIDGE_H
#define SHAREDMEMORYBRIDGE_H

#include <cstdint>
#include <windows.h>  // For HANDLE

struct ID3D11Texture2D;
struct ID3D11Device;

class SharedMemoryBridge {
public:
    SharedMemoryBridge();
    ~SharedMemoryBridge();

    bool Initialize();
    void Shutdown();

    // Create shared handle from DX11 texture
    bool CreateSharedHandle(ID3D11Texture2D* texture, ID3D11Device* device, HANDLE* outHandle);
    void TransferFrame();

private:
    bool initialized_ = false;
    HANDLE sharedHandle_ = nullptr;
};

#endif // SHAREDMEMORYBRIDGE_H
