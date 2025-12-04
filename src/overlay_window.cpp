// OverlayWindow Implementation - Transparent overlay rendering
#include "overlay_window.h"
#include <iostream>
#include <dwmapi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwmapi.lib")

OverlayWindow::OverlayWindow() = default;

OverlayWindow::~OverlayWindow() {
    Shutdown();
}

bool OverlayWindow::Initialize(HWND targetWindow, UINT width, UINT height) {
    if (m_initialized) {
        return true;
    }

    m_targetWindow = targetWindow;
    m_width = width;
    m_height = height;

    if (!CreateOverlayWindow() || !InitializeD3D11() || !InitializeD2D1()) {
        Shutdown();
        return false;
    }

    m_initialized = true;
    UpdateWindowPosition();

    std::cout << "[OverlayWindow] Initialized " << width << "x" << height << "\n";
    return true;
}

void OverlayWindow::Shutdown() {
    if (m_overlayWindow) {
        DestroyWindow(m_overlayWindow);
        m_overlayWindow = nullptr;
    }

    m_interpolatedBitmap.Reset();
    m_renderTarget.Reset();
    m_d2dFactory.Reset();
    m_d3dContext.Reset();
    m_d3dDevice.Reset();

    m_initialized = false;
}

bool OverlayWindow::CreateOverlayWindow() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"CrossLinkOverlay";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc)) {
        std::cerr << "[OverlayWindow] Failed to register window class\n";
        return false;
    }

    // Create transparent, click-through window
    m_overlayWindow = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        L"CrossLinkOverlay", L"CrossLink FG Overlay",
        WS_POPUP | WS_VISIBLE,
        0, 0, m_width, m_height,
        NULL, NULL, wc.hInstance, nullptr
    );

    if (!m_overlayWindow) {
        std::cerr << "[OverlayWindow] Failed to create overlay window\n";
        return false;
    }

    // Make it truly transparent and click-through
    SetLayeredWindowAttributes(m_overlayWindow, RGB(0, 0, 0), 255, LWA_ALPHA);

    // Extend frame into client area for transparency
    DwmExtendFrameIntoClientArea(m_overlayWindow, &MARGINS{ -1, -1, -1, -1 });

    return true;
}

bool OverlayWindow::InitializeD3D11() {
    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL featureLevel;

    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                createDeviceFlags, featureLevels, _countof(featureLevels),
                                D3D11_SDK_VERSION, &m_d3dDevice, &featureLevel, &m_d3dContext))) {
        std::cerr << "[OverlayWindow] Failed to create D3D11 device\n";
        return false;
    }

    return true;
}

bool OverlayWindow::InitializeD2D1() {
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2dFactory))) {
        std::cerr << "[OverlayWindow] Failed to create D2D1 factory\n";
        return false;
    }

    // Create HwndRenderTarget
    D2D1_RENDER_TARGET_PROPERTIES rtProps = {};
    rtProps.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
    rtProps.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
    rtProps.dpiX = 96.0f;
    rtProps.dpiY = 96.0f;

    D2D1_HWND_RENDER_TARGET_PROPERTIES hwndRtProps = {};
    hwndRtProps.hwnd = m_overlayWindow;
    hwndRtProps.pixelSize = D2D1::SizeU(m_width, m_height);

    if (FAILED(m_d2dFactory->CreateHwndRenderTarget(rtProps, hwndRtProps, &m_renderTarget))) {
        std::cerr << "[OverlayWindow] Failed to create D2D1 render target\n";
        return false;
    }

    return true;
}

bool OverlayWindow::RenderFrame(const void* frameData, UINT pitch) {
    if (!m_initialized || !m_renderTarget) {
        return false;
    }

    // Create bitmap from frame data
    D2D1_BITMAP_PROPERTIES bitmapProps = {};
    bitmapProps.pixelFormat = m_renderTarget->GetPixelFormat();
    bitmapProps.dpiX = 96.0f;
    bitmapProps.dpiY = 96.0f;

    // Recreate bitmap if needed
    if (!m_interpolatedBitmap) {
        if (FAILED(m_renderTarget->CreateBitmap(D2D1::SizeU(m_width, m_height),
                                               frameData, pitch, bitmapProps, &m_interpolatedBitmap))) {
            std::cerr << "[OverlayWindow] Failed to create bitmap\n";
            return false;
        }
    }

    // Copy frame data to bitmap
    D2D1_RECT_U rect = D2D1::RectU(0, 0, m_width, m_height);
    if (FAILED(m_interpolatedBitmap->CopyFromMemory(&rect, frameData, pitch))) {
        std::cerr << "[OverlayWindow] Failed to copy frame data\n";
        return false;
    }

    return true;
}

void OverlayWindow::Present() {
    if (!m_initialized || !m_renderTarget) {
        return;
    }

    m_renderTarget->BeginDraw();

    // Clear to transparent
    m_renderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Transparent));

    if (m_interpolatedBitmap) {
        D2D1_RECT_F rect = D2D1::RectF(0, 0, (float)m_width, (float)m_height);
        m_renderTarget->DrawBitmap(m_interpolatedBitmap.Get(), rect);
    }

    if (SUCCEEDED(m_renderTarget->EndDraw())) {
        ValidateRect(m_overlayWindow, nullptr);
    }
}

void OverlayWindow::SetTargetWindow(HWND window) {
    m_targetWindow = window;
    UpdateWindowPosition();
}

void OverlayWindow::UpdatePosition() {
    if (!m_targetWindow || !m_overlayWindow) {
        return;
    }

    RECT windowRect;
    if (GetWindowRect(m_targetWindow, &windowRect)) {
        MoveWindow(m_overlayWindow, windowRect.left, windowRect.top,
                  windowRect.right - windowRect.left,
                  windowRect.bottom - windowRect.top, TRUE);
    }
}

bool OverlayWindow::IsVisible() const {
    return m_initialized && IsWindowVisible(m_overlayWindow);
}

void OverlayWindow::UpdateWindowPosition() {
    UpdatePosition();
}

LRESULT CALLBACK OverlayWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT:
        ValidateRect(hwnd, nullptr);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}
