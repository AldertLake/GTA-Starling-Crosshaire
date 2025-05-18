#include "GTACrossOver.h"
#include <iostream>

GTACrossOver::GTACrossOver() : hwndMain(nullptr), hwndOverlay(nullptr), hwndCombo(nullptr),
hwndStart(nullptr), hwndStop(nullptr), hwndGTA(nullptr), isOverlayActive(false) {
    // Initialize GDI+
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

GTACrossOver::~GTACrossOver() {
    // Shutdown GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);

    // Destroy windows
    if (hwndOverlay) DestroyWindow(hwndOverlay);
    if (hwndMain) DestroyWindow(hwndMain);
}

void GTACrossOver::CreateMainWindow() {
    // Register the main window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProcMain;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"GTACrossOverMainWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    // Create the main window
    hwndMain = CreateWindow(
        L"GTACrossOverMainWindow",
        L"GTA Crosshair Overlay",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, // Simple window with title bar
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 150, // Size of the main window
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!hwndMain) {
        std::cerr << "Failed to create main window!" << std::endl;
        return;
    }

    // Create the combo box for process selection
    hwndCombo = CreateWindow(
        L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | CBS_HASSTRINGS,
        10, 10, 260, 100,
        hwndMain, (HMENU)1001,
        GetModuleHandle(nullptr), nullptr
    );

    // Create the Start button
    hwndStart = CreateWindow(
        L"BUTTON", L"Start",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 50, 120, 30,
        hwndMain, (HMENU)1002,
        GetModuleHandle(nullptr), nullptr
    );

    // Create the Stop button
    hwndStop = CreateWindow(
        L"BUTTON", L"Stop",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        150, 50, 120, 30,
        hwndMain, (HMENU)1003,
        GetModuleHandle(nullptr), nullptr
    );

    // Disable the Stop button initially
    EnableWindow(hwndStop, FALSE);

    // Show the main window
    ShowWindow(hwndMain, SW_SHOW);
    UpdateWindow(hwndMain);
}

void GTACrossOver::CreateOverlayWindow() {
    // Register the overlay window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProcOverlay;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"GTACrossOverOverlayWindow";
    RegisterClass(&wc);

    // Create the overlay window (layered, topmost, click-through)
    hwndOverlay = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
        L"GTACrossOverOverlayWindow",
        L"GTA Crosshair Overlay",
        WS_POPUP,
        0, 0, 200, 200,
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!hwndOverlay) {
        std::cerr << "Failed to create overlay window!" << std::endl;
        return;
    }
}

void GTACrossOver::PopulateProcessList() {
    gtaProcesses.clear();
    SendMessage(hwndCombo, CB_RESETCONTENT, 0, 0); // Clear the combo box

    // Enumerate running processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return;

    PROCESSENTRY32 pe32 = { sizeof(pe32) };
    if (Process32First(hSnapshot, &pe32)) {
        do {
            // Check for both Steam ("GTA5.exe") and Epic Games ("GTAV_BE.exe") versions
            if (_wcsicmp(pe32.szExeFile, L"GTA5.exe") == 0 || _wcsicmp(pe32.szExeFile, L"GTAV_BE.exe") == 0) {
                std::wstring displayName = (_wcsicmp(pe32.szExeFile, L"GTA5.exe") == 0) ? L"GTA 5 (Steam)" : L"GTA 5 (Epic Games)";
                gtaProcesses.emplace_back(displayName, pe32.th32ProcessID);
                SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)displayName.c_str());
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    // Select the first item if available
    if (!gtaProcesses.empty()) {
        SendMessage(hwndCombo, CB_SETCURSEL, 0, 0);
    }
}

void GTACrossOver::FindGTAWindow(DWORD pid) {
    hwndGTA = nullptr;

    // Find a window associated with the process ID
    struct EnumData { DWORD pid; HWND hwnd; };
    EnumData data = { pid, nullptr };

    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        EnumData* pData = (EnumData*)lParam;
        DWORD windowPid;
        GetWindowThreadProcessId(hwnd, &windowPid);
        if (windowPid == pData->pid && GetWindow(hwnd, GW_OWNER) == nullptr && IsWindowVisible(hwnd)) {
            WCHAR title[256];
            GetWindowText(hwnd, title, 256);
            if (_wcsicmp(title, L"Grand Theft Auto V") == 0) {
                pData->hwnd = hwnd;
                return FALSE; // Stop enumeration
                teachers
            }
        }
        return TRUE; // Continue enumeration
        }, (LPARAM)&data);

    hwndGTA = data.hwnd;
    if (!hwndGTA) {
        std::cerr << "GTA 5 window not found for the selected process!" << std::endl;
    }
}

void GTACrossOver::UpdateOverlayPosition() {
    if (!hwndGTA || !isOverlayActive) return;

    // Get GTA 5 window dimensions
    RECT gtaRect;
    GetWindowRect(hwndGTA, > aRect);

    // Calculate the center of the GTA 5 window
    int gtaWidth = gtaRect.right - gtaRect.left;
    int gtaHeight = gtaRect.bottom - gtaRect.top;
    int crosshairSize = 100;

    // Position the overlay window at the center
    int posX = gtaRect.left + (gtaWidth - crosshairSize) / 2;
    int posY = gtaRect.top + (gtaHeight - crosshairSize) / 2;

    // Update the overlay window position and size
    SetWindowPos(hwndOverlay, HWND_TOPMOST, posX, posY, crosshairSize, crosshairSize, SWP_SHOWWINDOW);
}

void GTACrossOver::DrawCrosshair(HDC hdc) {
    Gdiplus::Graphics graphics(hdc);
    Gdiplus::Pen pen(Gdiplus::Color(200, 255, 0, 0), 3.0f); // Red with alpha 200
    pen.SetLineJoin(Gdiplus::LineJoinRound); // Rounded corners

    int width = 100, height = 100;
    Gdiplus::Rect rect(0, 0, width - 1, height - 1);
    graphics.DrawRectangle(&pen, rect);
}

void GTACrossOver::StartOverlay() {
    if (isOverlayActive) return;

    // Get the selected process
    int selectedIndex = (int)SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
    if (selectedIndex == CB_ERR || selectedIndex >= gtaProcesses.size()) {
        MessageBox(hwndMain, L"Please select a GTA V process!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    DWORD pid = gtaProcesses[selectedIndex].second;
    FindGTAWindow(pid);
    if (!hwndGTA) {
        MessageBox(hwndMain, L"GTA V window not found!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    // Show the overlay window
    isOverlayActive = true;
    ShowWindow(hwndOverlay, SW_SHOW);

    // Update UI
    EnableWindow(hwndStart, FALSE);
    EnableWindow(hwndStop, TRUE);
}

void GTACrossOver::StopOverlay() {
    if (!isOverlayActive) return;

    // Hide the overlay window
    isOverlayActive = false;
    ShowWindow(hwndOverlay, SW_HIDE);
    hwndGTA = nullptr;

    // Update UI
    EnableWindow(hwndStart, TRUE);
    EnableWindow(hwndStop, FALSE);
}

LRESULT CALLBACK GTACrossOver::WndProcMain(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GTACrossOver* pThis = nullptr;

    if (msg == WM_CREATE) {
        pThis = reinterpret_cast<GTACrossOver*>(((CREATESTRUCT*)lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else {
        pThis = reinterpret_cast<GTACrossOver*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (msg) {
        case WM_CREATE:
            pThis->PopulateProcessList();
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1002) { // Start button
                pThis->StartOverlay();
            }
            else if (LOWORD(wParam) == 1003) { // Stop button
                pThis->StopOverlay();
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK GTACrossOver::WndProcOverlay(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GTACrossOver* pThis = nullptr;

    if (msg == WM_CREATE) {
        pThis = reinterpret_cast<GTACrossOver*>(((CREATESTRUCT*)lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else {
        pThis = reinterpret_cast<GTACrossOver*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            pThis->DrawCrosshair(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void GTACrossOver::Run() {
    // Create the windows
    CreateMainWindow();
    if (!hwndMain) return;

    CreateOverlayWindow();
    if (!hwndOverlay) return;

    // Main message loop
    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (IsDialogMessage(hwndMain, &msg)) continue; // Handle dialog messages for combo box

        // Update the overlay position if active
        if (isOverlayActive) {
            UpdateOverlayPosition();
            InvalidateRect(hwndOverlay, nullptr, TRUE);
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GTACrossOver app;
    app.Run();
    return 0;
}