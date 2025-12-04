#include <windows.h>
#include <iostream>
#include <string>
#include <filesystem>

int main(int argc, char* argv[]) {
    std::cout << "DEBUG: Main started - Entry point reached" << std::endl;

    try {
        std::cout << "DEBUG: Try-catch block entered" << std::endl;

        // Architecture detection
        std::cout << "CrossLink FG Loader" << std::endl;
        std::cout << "==================" << std::endl;
        if (sizeof(void*) == 8) {
            std::cout << "Architecture: x64 (64-bit)" << std::endl;
        } else {
            std::cout << "ERROR: Architecture: x86 (32-bit) - This will fail on 64-bit processes!" << std::endl;
        }
        std::cout << std::endl;

        std::string gamePath;

        // Check for command line argument
        if (argc > 1) {
            std::cout << "DEBUG: Received command line argument: " << argv[1] << std::endl;
            gamePath = argv[1];
        } else {
            std::cout << "DEBUG: No command line argument - waiting for user input" << std::endl;
            std::cout << "Enter path to game executable: ";
            std::getline(std::cin, gamePath);

            // Remove quotes if present
            if (!gamePath.empty() && gamePath[0] == '"') {
                gamePath = gamePath.substr(1, gamePath.size() - 2);
            }
        }

    // Check if file exists
    if (!std::filesystem::exists(gamePath)) {
        std::cerr << "Error: Game executable not found: " << gamePath << std::endl;
        return 1;
    }

    // Get the directory where the loader executable is located
    char exePath[MAX_PATH];
    if (!GetModuleFileNameA(NULL, exePath, MAX_PATH)) {
        std::cerr << "Error: Could not get executable path" << std::endl;
        return 1;
    }

    std::filesystem::path loaderPath = std::filesystem::path(exePath).parent_path();
    std::filesystem::path dllPath = loaderPath / "CrossLinkFG.dll";

    if (!std::filesystem::exists(dllPath)) {
        std::cerr << "Error: CrossLinkFG.dll not found in: " << dllPath.string() << std::endl;
        return 1;
    }

    std::cout << "Launching game: " << gamePath << std::endl;
    std::cout << "Injecting DLL: " << dllPath << std::endl;

    // Create process in suspended state
    STARTUPINFOA si = {0};
    PROCESS_INFORMATION pi = {0};
    si.cb = sizeof(si);

    std::string commandLine = "\"" + gamePath + "\"";

    if (!CreateProcessA(NULL, const_cast<char*>(commandLine.c_str()), NULL, NULL,
                       FALSE, CREATE_SUSPENDED, NULL, NULL, &si, &pi)) {
        std::cerr << "Error: Failed to create process. Error code: " << GetLastError() << std::endl;
        return 1;
    }

    std::cout << "Game process created in suspended state" << std::endl;

    // Allocate memory in target process for DLL path
    std::string dllPathStr = dllPath.string();
    size_t pathSize = (dllPathStr.size() + 1) * sizeof(char);

    LPVOID remoteMemory = VirtualAllocEx(pi.hProcess, NULL, pathSize,
                                        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!remoteMemory) {
        std::cerr << "Error: Failed to allocate memory in target process" << std::endl;
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 1;
    }

    // Write DLL path to target process
    if (!WriteProcessMemory(pi.hProcess, remoteMemory, dllPathStr.c_str(), pathSize, NULL)) {
        std::cerr << "Error: Failed to write to target process memory" << std::endl;
        VirtualFreeEx(pi.hProcess, remoteMemory, 0, MEM_RELEASE);
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 1;
    }

    // Get LoadLibraryA address
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    LPVOID loadLibraryAddr = (LPVOID)GetProcAddress(kernel32, "LoadLibraryA");

    if (!loadLibraryAddr) {
        std::cerr << "Error: Failed to get LoadLibraryA address" << std::endl;
        VirtualFreeEx(pi.hProcess, remoteMemory, 0, MEM_RELEASE);
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 1;
    }

    // DEBUG: Print absolute path before injection
    std::cout << "Injecting DLL with absolute path: " << dllPathStr << std::endl;

    // Create remote thread to load DLL
    HANDLE hThread = CreateRemoteThread(pi.hProcess, NULL, 0,
                                       (LPTHREAD_START_ROUTINE)loadLibraryAddr,
                                       remoteMemory, 0, NULL);

    if (!hThread) {
        std::cerr << "Error: Failed to create remote thread for DLL injection" << std::endl;
        VirtualFreeEx(pi.hProcess, remoteMemory, 0, MEM_RELEASE);
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 1;
    }

    // Wait for DLL to load
    WaitForSingleObject(hThread, INFINITE);

    // Check thread exit code
    DWORD exitCode;
    GetExitCodeThread(hThread, &exitCode);

    if (exitCode == 0) {
        std::cerr << "Error: DLL injection failed - LoadLibrary returned NULL" << std::endl;
        VirtualFreeEx(pi.hProcess, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(hThread);
        TerminateProcess(pi.hProcess, 1);
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
        return 1;
    }

    // Free allocated memory and close thread handle
    VirtualFreeEx(pi.hProcess, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(hThread);

    // Resume game
    ResumeThread(pi.hThread);

    std::cout << "Injection Successful!" << std::endl;
    std::cout << "CrossLink FG is now active in the game" << std::endl;
    std::cout << "You should see 'CrossLink FG: Active' overlay" << std::endl;

    // Keep handles open for cleanup on exit
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    // Debug: Wait for user input before exiting
    std::cout << std::endl << "Press Enter to exit..." << std::endl;
    std::cin.ignore();

    return 0;
    } catch (const std::exception& e) {
        std::cerr << "CRITICAL ERROR: " << e.what() << std::endl;
        std::cout << std::endl << "PRESS ENTER TO EXIT..." << std::endl;
        std::cin.get();
        return 1;
    } catch (...) {
        std::cerr << "UNKNOWN CRITICAL ERROR occurred!" << std::endl;
        std::cout << std::endl << "PRESS ENTER TO EXIT..." << std::endl;
        std::cin.get();
        return 1;
    }
}
