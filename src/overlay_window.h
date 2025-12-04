// OverlayWindow - Transparent Click-Through Overlay
// Renders interpolated frames on top of target window

#pragma once
#include <d2d1.h>
#include <d3d11.h>
#include <dxgi1_5.h>
#include <wrl/client.h>
#include <windows.h>

using Microsoft::WRL::ComPtr;

class OverlayWindow {
public:
    OverlayWindow();
    ~OverlayWindow();

    bool Initialize(HWND targetWindow, UINT width, UINT height);
    void Shutdown();

    // Frame rendering
    bool RenderFrame(const void* frameData, UINT pitch);
    void Present();

    // Window management
    void SetTargetWindow(HWND window);
    void UpdatePosition(); // Track target window position
    bool IsVisible() const;

    // Properties
    UINT GetWidth() const { return m_width; }
    UINT GetHeight() const { return m_height; }
    HWND GetOverlayHWND() const { return m_overlayWindow; }

private:
    // Window handles
    HWND m_targetWindow = nullptr;
    HWND m_overlayWindow = nullptr;

    // D3D11 Device for texture creation
    ComPtr<ID3D11Device> m_d3dDevice;
    ComPtr<ID3D11DeviceContext> m_d3dContext;

    // D2D1 for rendering
    ComPtr<ID2D1Factory> m_d2dFactory;
    ComPtr<ID2D1RenderTarget> m_renderTarget;
    ComPtr<ID2D1Bitmap> m_interpolatedBitmap;

    // Dimensions
    UINT m_width = 1920;
    UINT m_height = 1080;

    // Window state
    bool m_initialized = false;

    // Window creation and management
    bool CreateOverlayWindow();
    bool InitializeD3D11();
    bool InitializeD2D1();
    void UpdateWindowPosition();

    // Message handler
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};
