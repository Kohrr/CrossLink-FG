#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

// Forward declarations für Hook-Funktionen
bool InstallDX12Hooks();
void UninstallDX12Hooks();

// CRITICAL: MainThread Funktion
DWORD WINAPI MainThread(LPVOID lpParam) {
    HMODULE hModule = (HMODULE)lpParam;

    // STEP 1: Console erstellen
    AllocConsole();
    FILE* fp_stdout;
    freopen_s(&fp_stdout, "CONOUT$", "w", stdout);

    // STEP 2: Willkommensnachricht
    std::cout << "=======================================" << std::endl;
    std::cout << "CrossLink FG - God-Tier Main Thread!" << std::endl;
    std::cout << "=======================================" << std::endl;
    std::cout << "DLL erfolgreich in Zielprozess geladen." << std::endl;
    std::cout << "Console angehängt und Output umgeleitet." << std::endl;
    std::cout << std::endl;

    // STEP 3: GOD-TIER - Warte 8 Sekunden für Spielstart (langsamere Systems / Overlays)
    std::cout << "[GOD-TIER] Warte 8 Sekunden für kompletten Spielstart..." << std::endl;
    for (int i = 8; i > 0; i--) {
        std::cout << "[GOD-TIER] Countdown: " << i << " Sekunden..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "[GOD-TIER] Spielstartzeit abgelaufen. Initialisiere Hooks..." << std::endl;
    std::cout << std::endl;

    // STEP 4: Hooks installieren
    if (InstallDX12Hooks()) {
        std::cout << "===========================================" << std::endl;
        std::cout << "[GOD-TIER SUCCESS] 🎯 ALLE HOOKS INSTALLIERT!" << std::endl;
        std::cout << "[GOD-TIER SUCCESS] ExecuteCommandLists Hook: AKTIV" << std::endl;
        std::cout << "[GOD-TIER SUCCESS] Present Hook: AKTIV" << std::endl;
        std::cout << "[GOD-TIER SUCCESS] 🎮 OVERLAY WIRD SICHTBAR SEIN!" << std::endl;
        std::cout << "===========================================" << std::endl;

        // Erfolgs-Log schreiben
        std::ofstream successlog("CrossLink_GodTier_Success.txt");
        successlog << "GOD-TIER HOOKS ERFOLGREICH INSTALLIERT!" << std::endl;
        successlog << "Zeit: " << __TIME__ << std::endl;
        successlog << "Datum: " << __DATE__ << std::endl;
        successlog << "Overlay wird in Borderlands 3 sichtbar sein!" << std::endl;
        successlog.close();

        std::cout << std::endl;
        std::cout << "🎮 BITTE JETZT BORDERLANDS 3 STARTEN!" << std::endl;
        std::cout << "🎮 Das Overlay wird automatisch erscheinen!" << std::endl;
        std::cout << std::endl;

        // Hauptthread am Leben halten
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(10));
            std::cout << "[GOD-TIER] Hooks aktiv - CrossLink FG läuft..." << std::endl;
        }
    } else {
        std::cout << "===========================================" << std::endl;
        std::cout << "[GOD-TIER FAILURE] ❌ Hook-Installation fehlgeschlagen!" << std::endl;
        std::cout << "[GOD-TIER FAILURE] CrossLink FG ist nicht aktiv." << std::endl;
        std::cout << "===========================================" << std::endl;

        // Fehler-Log schreiben
        std::ofstream faillog("CrossLink_GodTier_Failure.txt");
        faillog << "GOD-TIER HOOK-INSTALLATION FEHLGESCHLAGEN!" << std::endl;
        faillog << "Zeit: " << __TIME__ << std::endl;
        faillog << "Datum: " << __DATE__ << std::endl;
        faillog.close();
    }

    return 0;
}

// DLL Entry Point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        // Kritisch: Thread-Aufrufe deaktivieren für Performance
        DisableThreadLibraryCalls(hModule);

        // Audio-Bestätigung: Beep bei erfolgreichem Laden
        MessageBeep(MB_OK);

        // Hauptthread starten - unsere Logik läuft dort
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

// Kompatibilitätsfunktionen für andere Module
bool InitializeHooks() {
    return InstallDX12Hooks();
}

void ShutdownHooks() {
    UninstallDX12Hooks();
}
