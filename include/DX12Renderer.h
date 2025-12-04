#ifndef DX12RENDERER_H
#define DX12RENDERER_H

#include <cstdint>

class DX12Renderer {
public:
    DX12Renderer();
    ~DX12Renderer();

    bool Initialize(uint32_t width, uint32_t height);
    void Shutdown();
    void Render();

private:
    bool initialized_ = false;
};

#endif // DX12RENDERER_H
