// IMGUI OVERLAY DLL - Separate from core hooks
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include "imgui_manager.h"

// Forward declarations for overlay hooks
bool InitializeOverlayHooks();
void ShutdownOverlayHooks();
DWORD WINAPI OverlayMainThread(LPVOID lpParam);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        // audible confirmation
        MessageBeep(MB_ICONASTERISK);

        // Start overlay thread
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)OverlayMainThread, hModule, 0, nullptr);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// Main overlay thread - waits for game and initializes ImGui
DWORD WINAPI OverlayMainThread(LPVOID lpParam) {
    HMODULE hModule = (HMODULE)lpParam;

    // Allocate console for overlay DLL too
    AllocConsole();
    FILE* fp_stdout;
    freopen_s(&fp_stdout, "CONOUT$", "w", stdout);

    std::cout << "=======================================" << std::endl;
    std::cout << "CrossLink FG OVERLAY - Main Thread" << std::endl;
    std::cout << "=======================================" << std::endl;

    // Wait for game startup
    std::cout << "Waiting for game initialization..." << std::endl;
    Sleep(5000); // 5 seconds for game to load

    // Initialize overlay hooks
    if (InitializeOverlayHooks()) {
        std::cout << "[OVERLAY SUCCESS] ImGui Overlay hooks initialized!" << std::endl;
        std::cout << "[OVERLAY SUCCESS] Visual overlay should now be visible." << std::endl;

        // Keep thread alive
        while (true) {
            Sleep(1000);
        }
    } else {
        std::cout << "[OVERLAY FAIL] Failed to initialize overlay hooks!" << std::endl;
    }

    return 0;
}
