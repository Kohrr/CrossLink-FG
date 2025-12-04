// AIPipeline Implementation - Coordinate all components
#include "ai_pipeline.h"
#include <iostream>
#include <chrono>

AIPipeline::AIPipeline() = default;

AIPipeline::~AIPipeline() {
    Shutdown();
}

bool AIPipeline::Initialize(HWND targetWindow) {
    m_targetWindow = targetWindow;
    if (!m_targetWindow) {
        FindTargetWindow(); // Auto-detect
    }

    if (!m_targetWindow) {
        std::cerr << "[AIPipeline] No target window found\n";
        return false;
    }

    // Initialize components
    if (!m_screenGrabber.Initialize()) {
        std::cerr << "[AIPipeline] ScreenGrabber init failed\n";
        return false;
    }

    if (!m_neuralInterpolator.Initialize()) {
        std::cerr << "[AIPipeline] NeuralInterpolator init failed\n";
        return false;
    }

    if (!m_overlayWindow.Initialize(m_targetWindow, m_screenGrabber.GetWidth(), m_screenGrabber.GetHeight())) {
        std::cerr << "[AIPipeline] OverlayWindow init failed\n";
        return false;
    }

    // Setup frame buffers
    if (!SetupFrameBuffers(m_screenGrabber.GetWidth(), m_screenGrabber.GetHeight())) {
        std::cerr << "[AIPipeline] Frame buffer setup failed\n";
        return false;
    }

    std::cout << "[AIPipeline] Initialized successfully - targeting window: " << GetWindowTitle(m_targetWindow) << "\n";
    return true;
}

void AIPipeline::Shutdown() {
    m_running = false;

    if (m_captureThread.joinable()) m_captureThread.join();
    if (m_inferenceThread.joinable()) m_inferenceThread.join();
    if (m_renderThread.joinable()) m_renderThread.join();

    m_overlayWindow.Shutdown();
    m_neuralInterpolator.Shutdown();
    m_screenGrabber.Shutdown();
}

void AIPipeline::Run() {
    m_running = true;

    // Start threads
    m_captureThread = std::thread(&AIPipeline::CaptureLoop, this);
    m_inferenceThread = std::thread(&AIPipeline::InferenceLoop, this);
    m_renderThread = std::thread(&AIPipeline::RenderLoop, this);

    std::cout << "[AIPipeline] All threads started. Frame generation active.\n";

    // Keep main thread alive
    while (m_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void AIPipeline::CaptureLoop() {
    while (m_running) {
        if (m_screenGrabber.AcquireNextFrame(nullptr)) {
            // Copy frame to buffer A or B
            // Simplified placeholder
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

void AIPipeline::InferenceLoop() {
    while (m_running) {
        if (CreateInterpolatedFrame()) {
            // Generated new interpolated frame
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(8)); // 120 FPS
    }
}

void AIPipeline::RenderLoop() {
    while (m_running) {
        m_overlayWindow.Present();
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
}

bool AIPipeline::SetupFrameBuffers(int width, int height) {
    size_t bufferSize = width * height * 4; // RGBA

    m_frameBufferA.resize(bufferSize);
    m_frameBufferB.resize(bufferSize);
    m_interpolatedBuffer.resize(bufferSize);

    return true;
}

bool AIPipeline::CreateInterpolatedFrame() {
    if (m_frameBufferA.empty() || m_frameBufferB.empty()) {
        return false;
    }

    return m_neuralInterpolator.GenerateInterpolation(
        m_frameBufferA.data(), m_frameBufferB.data(),
        m_interpolatedBuffer.data(),
        m_screenGrabber.GetWidth(), m_screenGrabber.GetHeight()
    );
}

void AIPipeline::FindTargetWindow() {
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));
}

BOOL CALLBACK AIPipeline::EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    AIPipeline* pipeline = reinterpret_cast<AIPipeline*>(lParam);

    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));

    // Look for common game windows - very basic for now
    if (strstr(title, "Borderlands") || strstr(title, "Game") || strstr(title, "DX")) {
        pipeline->m_targetWindow = hwnd;
        return FALSE; // Stop enumeration
    }

    return TRUE; // Continue
}

std::string AIPipeline::GetWindowTitle(HWND hwnd) {
    char title[256];
    GetWindowTextA(hwnd, title, sizeof(title));
    return title;
}
