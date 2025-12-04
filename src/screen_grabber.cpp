// ScreenGrabber Implementation - DXGI Desktop Duplication
#include "screen_grabber.h"
#include <iostream>
#include <chrono>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

ScreenGrabber::ScreenGrabber() = default;

ScreenGrabber::~ScreenGrabber() {
    Shutdown();
}

bool ScreenGrabber::Initialize() {
    if (m_initialized) {
        return true;
    }

    if (!CreateDeviceAndDuplication()) {
        std::cerr << "[ScreenGrabber] Failed to create DXGI duplication\n";
        return false;
    }

    // Create staging texture for CPU access
    D3D11_TEXTURE2D_DESC stagingDesc = {};
    stagingDesc.Width = m_width;
    stagingDesc.Height = m_height;
    stagingDesc.MipLevels = 1;
    stagingDesc.ArraySize = 1;
    stagingDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    stagingDesc.SampleDesc.Count = 1;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.BindFlags = 0;
    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    stagingDesc.MiscFlags = 0;

    if (FAILED(m_device->CreateTexture2D(&stagingDesc, nullptr, &m_stagingTex))) {
        std::cerr << "[ScreenGrabber] Failed to create staging texture\n";
        return false;
    }

    m_initialized = true;
    std::cout << "[ScreenGrabber] Initialized " << m_width << "x" << m_height << "\n";
    return true;
}

void ScreenGrabber::Shutdown() {
    if (m_stagingTex) m_stagingTex.Reset();
    if (m_duplication) m_duplication.Reset();
    if (m_context) m_context.Reset();
    if (m_device) m_device.Reset();

    m_initialized = false;
    m_frameCount = 0;
}

bool ScreenGrabber::CreateDeviceAndDuplication() {
    // Create D3D11 device
    D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
                                0, featureLevels, _countof(featureLevels),
                                D3D11_SDK_VERSION, &m_device, nullptr, &m_context))) {
        std::cerr << "[ScreenGrabber] Failed to create D3D11 device\n";
        return false;
    }

    // Get DXGI Adapter
    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IDXGIAdapter> adapter;
    ComPtr<IDXGIOutput> output;

    if (FAILED(m_device.As(&dxgiDevice)) ||
        FAILED(dxgiDevice->GetAdapter(&adapter)) ||
        FAILED(adapter->EnumOutputs(0, &output))) {
        std::cerr << "[ScreenGrabber] Failed to get DXGI adapter/output\n";
        return false;
    }

    // Get output description
    DXGI_OUTPUT_DESC outputDesc;
    output->GetDesc(&outputDesc);
    m_width = outputDesc.DesktopCoordinates.right - outputDesc.DesktopCoordinates.left;
    m_height = outputDesc.DesktopCoordinates.bottom - outputDesc.DesktopCoordinates.top;

    // Create duplication
    ComPtr<IDXGIOutput4> output4;
    if (FAILED(output.As(&output4))) {
        std::cerr << "[ScreenGrabber] IDXGIOutput4 not supported\n";
        return false;
    }

    if (FAILED(output4->DuplicateOutput(m_device.Get(), &m_duplication))) {
        std::cerr << "[ScreenGrabber] Failed to create output duplication\n";
        return false;
    }

    return true;
}

bool ScreenGrabber::AcquireNextFrame(HANDLE sharedHandle) {
    if (!m_initialized || !m_duplication) {
        return false;
    }

    DXGI_OUTDUPL_DESC duplicationDesc;
    m_duplication->GetDesc(&duplicationDesc);

    // Release previous frame
    if (m_frameCount > 0) {
        m_duplication->ReleaseFrame();
    }

    // Acquire next frame
    ComPtr<IDXGIResource> desktopResource;
    HRESULT hr = m_duplication->AcquireNextFrame(0, &m_frameInfo, &desktopResource);

    if (FAILED(hr)) {
        if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
            return false; // No new frame
        } else if (hr == DXGI_ERROR_ACCESS_LOST) {
            // Recreate duplication
            CreateDeviceAndDuplication();
            return false;
        } else {
            std::cerr << "[ScreenGrabber] AcquireNextFrame failed: " << std::hex << hr << "\n";
            return false;
        }
    }

    // Get desktop texture
    ComPtr<ID3D11Texture2D> desktopTexture;
    if (FAILED(desktopResource.As(&desktopTexture))) {
        std::cerr << "[ScreenGrabber] Failed to get desktop texture\n";
        m_duplication->ReleaseFrame();
        return false;
    }

    // Copy to staging texture for CPU access
    m_context->CopyResource(m_stagingTex.Get(), desktopTexture.Get());

    // Create shared handle (optional for inter-process)
    if (sharedHandle) {
        // This would involve shared resources - simplified for now
    }

    m_frameCount++;
    return true;
}

bool ScreenGrabber::IsFrameAvailable() const {
    return m_frameInfo.LastPresentTime.QuadPart != 0;
}

void ScreenGrabber::SetTargetWindow(HWND window) {
    // TODO: Implement window-specific capture instead of desktop
    // For now, just adjust dimensions if possible
    RECT rect;
    if (GetWindowRect(window, &rect)) {
        m_width = rect.right - rect.left;
        m_height = rect.bottom - rect.top;
        std::cout << "[ScreenGrabber] Target window set " << m_width << "x" << m_height << "\n";
    }
}

void ScreenGrabber::ProcessDirtyRects(const std::vector<RECT>& dirtyRects) {
    // TODO: Implement dirty rect processing for optimization
    // Only update changed regions
}
