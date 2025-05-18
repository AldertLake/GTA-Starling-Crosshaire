#include "GTACrossOver.h"
#include <iostream>

GTACrossOver::GTACrossOver() : hwndMain(nullptr), hwndStatus(nullptr),
hwndStart(nullptr), hwndStop(nullptr), hwndModeSelect(nullptr), hwndGTA(nullptr),
isOverlayActive(false), isGTARunning(false), currentMode(DisplayMode::BorderlessWindowed) {
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    std::cout << "GTACrossOver constructed" << std::endl;
}

GTACrossOver::~GTACrossOver() {
    Gdiplus::GdiplusShutdown(gdiplusToken);
    if (hwndMain) DestroyWindow(hwndMain);
}

void GTACrossOver::CreateMainWindow() {
    std::cout << "Creating main window..." << std::endl;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProcMain;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"GTACrossOverMainWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wc);

    hwndMain = CreateWindow(
        L"GTACrossOverMainWindow",
        L"GTA V Overlay",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, 300, 200, // Height adjusted for dropdown
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!hwndMain) {
        std::cerr << "Failed to create main window!" << std::endl;
        return;
    }

    std::cout << "Main window created successfully: " << hwndMain << std::endl;

    hwndStatus = CreateWindow(
        L"STATIC", L"GTA V Detected: Not Running",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, 10, 260, 20,
        hwndMain, (HMENU)1000,
        GetModuleHandle(nullptr), nullptr
    );

    // Restore display mode selection
    CreateWindow(
        L"STATIC", L"Display Mode:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        10, 40, 100, 20,
        hwndMain, nullptr,
        GetModuleHandle(nullptr), nullptr
    );

    hwndModeSelect = CreateWindow(
        L"COMBOBOX", nullptr,
        CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL,
        110, 40, 160, 100,
        hwndMain, (HMENU)1003,
        GetModuleHandle(nullptr), nullptr
    );

    SendMessage(hwndModeSelect, CB_ADDSTRING, 0, (LPARAM)L"Borderless Windowed");
    SendMessage(hwndModeSelect, CB_ADDSTRING, 0, (LPARAM)L"Windowed");
    SendMessage(hwndModeSelect, CB_SETCURSEL, 0, 0);

    hwndStart = CreateWindow(
        L"BUTTON", L"Start",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 80, 120, 30,
        hwndMain, (HMENU)1001,
        GetModuleHandle(nullptr), nullptr
    );

    hwndStop = CreateWindow(
        L"BUTTON", L"Stop",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        150, 80, 120, 30,
        hwndMain, (HMENU)1002,
        GetModuleHandle(nullptr), nullptr
    );

    EnableWindow(hwndStart, FALSE);
    EnableWindow(hwndStop, FALSE);

    ShowWindow(hwndMain, SW_SHOW);
    UpdateWindow(hwndMain);

    SetTimer(hwndMain, 1, 1000, nullptr);
}

void GTACrossOver::UpdateGTAStatus() {
    DWORD pid = FindGTAProcess();
    isGTARunning = (pid != 0);

    std::wstring statusText = isGTARunning ? L"GTA V Detected: Running" : L"GTA V Detected: Not Running";
    SetWindowText(hwndStatus, statusText.c_str());
    std::cout << "GTA V Status: " << (isGTARunning ? "Running" : "Not Running") << std::endl;

    EnableWindow(hwndStart, isGTARunning && !isOverlayActive);
    EnableWindow(hwndStop, isOverlayActive);

    if (isOverlayActive) {
        std::cout << "Overlay is active, updating position..." << std::endl;
        overlayDrawer.UpdateOverlayPosition(hwndGTA, currentMode);
    }
}

DWORD GTACrossOver::FindGTAProcess() {
    DWORD pid = 0;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32 pe32 = { sizeof(pe32) };
    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (_wcsicmp(pe32.szExeFile, L"GTA5.exe") == 0 || _wcsicmp(pe32.szExeFile, L"GTAV_BE.exe") == 0) {
                pid = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);
    return pid;
}

void GTACrossOver::FindGTAWindow(DWORD pid) {
    hwndGTA = nullptr;

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
                return FALSE;
            }
        }
        return TRUE;
        }, (LPARAM)&data);

    hwndGTA = data.hwnd;
    if (hwndGTA) {
        std::cout << "GTA V window found: " << hwndGTA << std::endl;
    }
    else {
        std::cerr << "GTA V window not found!" << std::endl;
    }
}

void GTACrossOver::StartOverlay() {
    if (isOverlayActive) return;

    DWORD pid = FindGTAProcess();
    if (pid == 0) {
        MessageBox(hwndMain, L"GTA V is not running!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    FindGTAWindow(pid);
    if (!hwndGTA) {
        MessageBox(hwndMain, L"GTA V window not found!", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    isOverlayActive = true;
    std::cout << "Starting overlay..." << std::endl;
    overlayDrawer.UpdateOverlayPosition(hwndGTA, currentMode);
    overlayDrawer.ShowOverlay();

    EnableWindow(hwndStart, FALSE);
    EnableWindow(hwndStop, TRUE);
}

void GTACrossOver::StopOverlay() {
    if (!isOverlayActive) return;

    isOverlayActive = false;
    std::cout << "Stopping overlay..." << std::endl;
    overlayDrawer.HideOverlay();
    hwndGTA = nullptr;

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
        case WM_TIMER:
            if (wParam == 1) {
                pThis->UpdateGTAStatus();
            }
            return 0;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1001) {
                pThis->StartOverlay();
            }
            else if (LOWORD(wParam) == 1002) {
                pThis->StopOverlay();
            }
            else if (LOWORD(wParam) == 1003 && HIWORD(wParam) == CBN_SELCHANGE) {
                LRESULT sel = SendMessage(pThis->hwndModeSelect, CB_GETCURSEL, 0, 0);
                pThis->currentMode = (sel == 0) ? DisplayMode::BorderlessWindowed : DisplayMode::Windowed;
                std::cout << "Display mode changed to: " << (pThis->currentMode == DisplayMode::BorderlessWindowed ? "Borderless Windowed" : "Windowed") << std::endl;
                if (pThis->isOverlayActive) {
                    pThis->overlayDrawer.UpdateOverlayPosition(pThis->hwndGTA, pThis->currentMode);
                }
            }
            return 0;
        case WM_DESTROY:
            KillTimer(hwnd, 1);
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void GTACrossOver::Run() {
    CreateMainWindow();
    if (!hwndMain) return;

    overlayDrawer.CreateOverlayWindow(GetModuleHandle(nullptr));

    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
    GTACrossOver app;
    app.Run();
    return 0;
}