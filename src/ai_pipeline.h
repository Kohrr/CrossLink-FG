// AIPipeline - Main Frame Generation Orchestrator
// Coordinates capture, AI inference, and overlay rendering

#pragma once
#include "screen_grabber.h"
#include "neural_interpolator.h"
#include "overlay_window.h"
#include <windows.h>
#include <thread>
#include <atomic>

class AIPipeline {
public:
    AIPipeline();
    ~AIPipeline();

    bool Initialize(HWND targetWindow = nullptr);
    void Shutdown();
    void Run();

    // Configuration
    void SetTargetFPS(float fps) { m_targetFPS = fps; }
    void SetInterpolationFactor(float factor) { m_interpolationFactor = factor; }

    // State
    bool IsRunning() const { return m_running; }

private:
    // Components
    ScreenGrabber m_screenGrabber;
    NeuralInterpolator m_neuralInterpolator;
    OverlayWindow m_overlayWindow;

    // Frame buffers
    std::vector<unsigned char> m_frameBufferA;
    std::vector<unsigned char> m_frameBufferB;
    std::vector<unsigned char> m_interpolatedBuffer;

    // Timing and sync
    float m_targetFPS = 60.0f;
    float m_interpolationFactor = 0.5f;
    std::atomic<bool> m_running{ false };

    // Window tracking
    HWND m_targetWindow = nullptr;

    // Pipeline threads
    std::thread m_captureThread;
    std::thread m_inferenceThread;
    std::thread m_renderThread;

    // Frame management
    bool m_hasFrameA = false;
    bool m_hasFrameB = false;

    // Pipeline methods
    void CaptureLoop();
    void InferenceLoop();
    void RenderLoop();

    bool SetupFrameBuffers(int width, int height);
    bool CreateInterpolatedFrame();
    void FindTargetWindow();

    // Utilities
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    std::string GetWindowTitle(HWND hwnd);
};
