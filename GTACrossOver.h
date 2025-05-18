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
    HWND hwndMain;   
    HWND hwndOverlay; 
    HWND hwndCombo;   
    HWND hwndStart;   
    HWND hwndStop;    
    HWND hwndGTA;     
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    bool isOverlayActive; 
    std::vector<std::pair<std::wstring, DWORD>> gtaProcesses; 

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