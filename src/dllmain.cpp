#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include "frame_processor.h"

// Forward declarations
bool InitializeHooks();
void ShutdownHooks();
DWORD WINAPI MainThread(LPVOID lpParam);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // MINIMAL: Just create thread and beep to confirm loading
        DisableThreadLibraryCalls(hModule);

        // AUDIBLE CONFIRMATION: Beep on successful load
        MessageBeep(MB_OK);

        // Start main thread - our actual logic runs there
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// MAIN THREAD - RUNS AFTER DLLMAIN RETURNS (thread-safe)
DWORD WINAPI MainThread(LPVOID lpParam) {
    HMODULE hModule = (HMODULE)lpParam;

    // STEP 1: Allocate console window
    AllocConsole();

    // STEP 2: Redirect stdout to console
    FILE* fp_stdout;
    freopen_s(&fp_stdout, "CONOUT$", "w", stdout);

    // STEP 3: Welcome message
    std::cout << "=======================================" << std::endl;
    std::cout << "CrossLink FG - Main Thread Started!" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "DLL successfully injected into target process." << std::endl;
    std::cout << "Console attached and output redirected." << std::endl;
    std::cout << std::endl;

    // STEP 4: WAIT for game startup to complete (critical!)
    std::cout << "Waiting 3 seconds for game startup to complete..." << std::endl;
    Sleep(3000);  // 3 seconds
    std::cout << "Game startup time elapsed. Initializing hooks..." << std::endl;
    std::cout << std::endl;

    // STEP 5: Start Frame Processor for async frame processing
    FrameProcessor::Start();
    std::cout << "[FRAME PROCESSOR] Started for Cross-GPU frame generation." << std::endl;

    // STEP 6: NOW it's safe to initialize hooks (game is running)
    if (InitializeHooks()) {
        std::cout << "=======================================" << std::endl;
        std::cout << "[SUCCESS] CrossLink FG hooks initialized!" << std::endl;
        std::cout << "[SUCCESS] Frame generation active in game." << std::endl;
        std::cout << "[SUCCESS] You should see the overlay now." << std::endl;
        std::cout << "=======================================" << std::endl;

        // Update debug file with success
        std::ofstream successlog("CrossLink_Success.txt");
        successlog << "HOOKS INITIALIZED SUCCESSFULLY!" << std::endl;
        successlog << "Time: " << __TIME__ << std::endl;
        successlog << "Date: " << __DATE__ << std::endl;
        successlog.close();

        // Keep thread alive
        while (true) {
            Sleep(1000);  // Sleep forever - keeps hooks alive
        }
    } else {
        std::cout << "=======================================" << std::endl;
        std::cout << "[FAILURE] Failed to initialize hooks!" << std::endl;
        std::cout << "[FAILURE] CrossLink FG not active." << std::endl;
        std::cout << "=======================================" << std::endl;

        // Update debug file with failure
        std::ofstream faillog("CrossLink_Failure.txt");
        faillog << "HOOK INITIALIZATION FAILED!" << std::endl;
        faillog << "Time: " << __TIME__ << std::endl;
        faillog.close();
    }

    return 0;
}
