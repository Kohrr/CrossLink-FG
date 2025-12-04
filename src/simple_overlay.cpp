// SIMPLE OVERLAY DLL - No DX12 complexity, just a basic information window
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <string>

// Simple overlay window
HWND g_overlayWindow = NULL;
HINSTANCE g_hInstance = NULL;
bool g_overlayVisible = false;

// Window procedure for overlay window
LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Simple text rendering
        RECT rect;
        GetClientRect(hwnd, &rect);

        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(0, 255, 0)); // Green text

        std::string statusText = "CrossLink FG Active\nFrame Generation Ready\n";
        DrawTextA(hdc, statusText.c_str(), -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE);
        g_overlayVisible = false;
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

// Toggle overlay visibility
void ToggleOverlay() {
    if (!g_overlayWindow) {
        // Create overlay window
        WNDCLASSEX wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = OverlayWndProc;
        wc.hInstance = g_hInstance;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wc.lpszClassName = TEXT("CrossLinkOverlay");

        if (!RegisterClassEx(&wc)) {
            std::cout << "[OVERLAY ERROR] Failed to register window class" << std::endl;
            return;
        }

        g_overlayWindow = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            TEXT("CrossLinkOverlay"),
            TEXT("CrossLink Frame Generation"),
            WS_POPUP,
            CW_USEDEFAULT, CW_USEDEFAULT,
            400, 200,
            NULL, NULL, g_hInstance, NULL
        );

        if (!g_overlayWindow) {
            std::cout << "[OVERLAY ERROR] Failed to create overlay window" << std::endl;
            return;
        }

        SetLayeredWindowAttributes(g_overlayWindow, 0, 192, LWA_ALPHA); // Semi-transparent

        std::cout << "[OVERLAY] Window created successfully" << std::endl;
    }

    g_overlayVisible = !g_overlayVisible;
    ShowWindow(g_overlayWindow, g_overlayVisible ? SW_SHOW : SW_HIDE);

    if (g_overlayVisible) {
        std::cout << "[OVERLAY] CrossLink FG overlay is now VISIBLE on screen!" << std::endl;

        // Move overlay to top-right corner
        RECT desktopRect;
        GetWindowRect(GetDesktopWindow(), &desktopRect);
        SetWindowPos(g_overlayWindow, HWND_TOPMOST,
                    desktopRect.right - 420, 20,
                    400, 200, SWP_NOZORDER);
    } else {
        std::cout << "[OVERLAY] CrossLink FG overlay is now HIDDEN" << std::endl;
    }
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        g_hInstance = hModule;
        DisableThreadLibraryCalls(hModule);

        // Audible confirmation
        MessageBeep(MB_ICONASTERISK);

        // Start initialization thread
        CreateThread(NULL, 0, [](LPVOID) -> DWORD {
            std::cout << "[SIMPLE OVERLAY] DLL loaded successfully" << std::endl;
            std::cout << "[SIMPLE OVERLAY] Press F1 to toggle CrossLink FG overlay" << std::endl;

            // Hook F1 key for toggle
            while (true) {
                if (GetAsyncKeyState(VK_F1) & 0x8000) {
                    ToggleOverlay();
                    Sleep(200); // Debounce
                }
                Sleep(10); // Don't hog CPU
            }

            return 0;
        }, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        if (g_overlayWindow) {
            DestroyWindow(g_overlayWindow);
            g_overlayWindow = NULL;
        }
        break;
    }

    return TRUE;
}
