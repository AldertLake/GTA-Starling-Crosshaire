#pragma once
#include <windows.h>
#include <gdiplus.h>

enum class DisplayMode {
    BorderlessWindowed,
    Windowed
};

class OverlayDrawer {
public:
    OverlayDrawer();
    ~OverlayDrawer();
    void CreateOverlayWindow(HINSTANCE hInstance);
    void UpdateOverlayPosition(HWND hwndTarget, DisplayMode mode); // Restore mode parameter
    void ShowOverlay();
    void HideOverlay();
    void SetCrosshairSize(int size);
    HWND GetOverlayHandle() const { return hwndOverlay; }

private:
    HWND hwndOverlay;
    int crosshairSize;
    HRGN lastRegion;
    static LRESULT CALLBACK WndProcOverlay(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void DrawDot(HDC hdc);
    void UpdateWindowRegion(HWND hwndTarget);
};