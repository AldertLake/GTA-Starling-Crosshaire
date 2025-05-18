#pragma once
#include "OverlayDrawer.h"
#include <windows.h>
#include <gdiplus.h>
#include <tlhelp32.h>
#include <string>

class GTACrossOver {
public:
    GTACrossOver();
    ~GTACrossOver();
    void CreateMainWindow();
    void UpdateGTAStatus();
    void StartOverlay();
    void StopOverlay();
    void Run();

private:
    HWND hwndMain;
    HWND hwndStatus;
    HWND hwndStart;
    HWND hwndStop;
    HWND hwndModeSelect; // Restore display mode selection
    HWND hwndGTA;
    bool isOverlayActive;
    bool isGTARunning;
    OverlayDrawer overlayDrawer;
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    DisplayMode currentMode; // Restore display mode

    DWORD FindGTAProcess();
    void FindGTAWindow(DWORD pid);
    static LRESULT CALLBACK WndProcMain(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};