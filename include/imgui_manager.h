#ifndef IMGUI_MANAGER_H
#define IMGUI_MANAGER_H

#include <Windows.h>
#include <d3d12.h>
#include <dxgi.h>
#include <imgui.h>

class ImGuiManager {
public:
    static bool Initialize(ID3D12Device* device, IDXGISwapChain* swapChain);
    static void Shutdown();
    static void NewFrame();
    static void Render(ID3D12GraphicsCommandList* commandList);
    static void RenderOverlay();
    static bool IsInitialized() { return s_initialized; }

    static ID3D12DescriptorHeap* GetSrvDescHeap() { return s_pd3dSrvDescHeap; }

    // Win32 Message Handler Hook
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Setter für die Queue (wichtig für DX12!)
    static void SetCommandQueue(ID3D12CommandQueue* queue) { s_pCommandQueue = queue; }

private:
    static bool s_initialized;
    static ID3D12DescriptorHeap* s_pd3dSrvDescHeap;
    static ID3D12CommandQueue* s_pCommandQueue;
    static HWND s_hwnd;
};

#endif // IMGUI_MANAGER_H
