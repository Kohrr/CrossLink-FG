// ScreenGrabber - DXGI Desktop Duplication
// Part of Universal Mode CrossLink-FG (Lossless Scaling Style)

#pragma once
#include <d3d11.h>
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <vector>

using Microsoft::WRL::ComPtr;

class ScreenGrabber {
public:
    ScreenGrabber();
    ~ScreenGrabber();

    bool Initialize();
    void Shutdown();

    // Capture desktop/window
    bool AcquireNextFrame(HANDLE sharedHandle);
    bool IsFrameAvailable() const;

    // Window targeting
    void SetTargetWindow(HWND window);
    void SetCaptureCursor(bool capture) { m_captureCursor = capture; }

    // Metadata
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }

private:
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGIOutputDuplication> m_duplication;
    ComPtr<ID3D11Texture2D> m_stagingTex;

    UINT m_width = 1920;
    UINT m_height = 1080;
    bool m_initialized = false;
    bool m_captureCursor = true;

    // Frame timing
    DXGI_OUTDUPL_FRAME_INFO m_frameInfo;
    uint32_t m_frameCount = 0;

    bool CreateDeviceAndDuplication();
    void ProcessDirtyRects(const std::vector<RECT>& dirtyRects);
};
