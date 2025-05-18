#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <tlhelp32.h>
#include <vector>
#include <string>
#pragma comment(lib, "gdiplus.lib")

class GTACrossOver {
public:
    GTACrossOver();
    ~GTACrossOver();
    void Run();

private:
    HWND hwndMain;    // Main window with UI
    HWND hwndOverlay; // Overlay window for crosshair
    HWND hwndCombo;   // Combo box for process selection
    HWND hwndStart;   // Start button
    HWND hwndStop;    // Stop button
    HWND hwndGTA;     // Handle to the GTA 5 window
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    bool isOverlayActive; // Tracks if the overlay is active
    std::vector<std::pair<std::wstring, DWORD>> gtaProcesses; // List of GTA V processes

    static LRESULT CALLBACK WndProcMain(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK WndProcOverlay(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void CreateMainWindow();
    void CreateOverlayWindow();
    void PopulateProcessList();
    void FindGTAWindow(DWORD pid);
    void UpdateOverlayPosition();
    void DrawCrosshair(HDC hdc);
    void StartOverlay();
    void StopOverlay();
};