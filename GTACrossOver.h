#pragma once
#include <windows.h>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

class GTACrossOver {
public:
    GTACrossOver();
    ~GTACrossOver();
    void Run();

private:
    HWND hwndOverlay; // Handle to the overlay window
    HWND hwndGTA;     // Handle to the GTA 5 window
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;

    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void CreateOverlayWindow();
    void FindGTAWindow();
    void UpdateOverlayPosition();
    void DrawCrosshair(HDC hdc);
};