// Universal Main - CrossLink-FG Standalone Frame Generator
// Lossless Scaling Style AI Frame Interpolation

#include "ai_pipeline.h"
#include <iostream>
#include <windows.h>
#include <signal.h>

std::unique_ptr<AIPipeline> g_pipeline;

// Console handler for clean shutdown
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrlType) {
    if (g_pipeline) {
        g_pipeline->Shutdown();
    }
    return TRUE;
}

int main(int argc, char* argv[]) {
    // Setup console
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    std::cout << "=========================================\n";
    std::cout << " CrossLink-FG Universal Frame Generator\n";
    std::cout << " Lossless Scaling AI Mode\n";
    std::cout << "=========================================\n";

    // Setup signal handlers
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    // Initialize pipeline
    g_pipeline = std::make_unique<AIPipeline>();
    if (!g_pipeline->Initialize(nullptr)) { // nullptr = auto-detect target window
        std::cerr << "[ERROR] Failed to initialize AI pipeline\n";
        return 1;
    }

    std::cout << "[MAIN] Starting frame generation...\n";

    // Run main loop
    g_pipeline->Run();

    std::cout << "[MAIN] Shutting down...\n";
    g_pipeline->Shutdown();

    std::cout << "[MAIN] Exited cleanly\n";
    return 0;
}
