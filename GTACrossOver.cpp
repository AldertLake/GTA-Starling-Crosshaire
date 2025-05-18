#include "GTACrossOver.h"
#include <iostream>

GTACrossOver::GTACrossOver() : hwndOverlay(nullptr), hwndGTA(nullptr) {
    // Initialize GDI+
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

GTACrossOver::~GTACrossOver() {
    // Shutdown GDI+
    Gdiplus::GdiplusShutdown(gdiplusToken);

    // Destroy the overlay window
    if (hwndOverlay) {
        DestroyWindow(hwndOverlay);
    }
}

void GTACrossOver::CreateOverlayWindow() {
    // Register the window class
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = L"GTACrossOverWindow";
    RegisterClass(&wc);

    // Create a layered window (for transparency)
    hwndOverlay = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT, // Extended style: layered, topmost, click-through
        L"GTACrossOverWindow",
        L"GTA Crosshair Overlay",
        WS_POPUP, // No window chrome
        0, 0, 200, 200, // Initial size/position (will be adjusted later)
        nullptr, nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!hwndOverlay) {
        std::cerr << "Failed to create overlay window!" << std::endl;
        return;
    }

    // Make the window initially invisible
    ShowWindow(hwndOverlay, SW_SHOW);
}

void GTACrossOver::FindGTAWindow() {
    // Find the GTA 5 window by its title
    hwndGTA = FindWindow(nullptr, L"Grand Theft Auto V");
    if (!hwndGTA) {
        std::cerr << "GTA 5 window not found!" << std::endl;
    }
}

void GTACrossOver::UpdateOverlayPosition() {
    if (!hwndGTA) return;

    // Get GTA 5 window dimensions
    RECT gtaRect;
    GetWindowRect(hwndGTA, &gtaRect);

    // Calculate the center of the GTA 5 window
    int gtaWidth = gtaRect.right - gtaRect.left;
    int gtaHeight = gtaRect.bottom - gtaRect.top;
    int crosshairSize = 100; // Size of the crosshair square

    // Position the overlay window at the center of the GTA 5 window
    int posX = gtaRect.left + (gtaWidth - crosshairSize) / 2;
    int posY = gtaRect.top + (gtaHeight - crosshairSize) / 2;

    // Update the overlay window position and size
    SetWindowPos(hwndOverlay, HWND_TOPMOST, posX, posY, crosshairSize, crosshairSize, SWP_SHOWWINDOW);
}

void GTACrossOver::DrawCrosshair(HDC hdc) {
    // Use GDI+ to draw the crosshair
    Gdiplus::Graphics graphics(hdc);

    // Create a slightly transparent red pen for the frame
    Gdiplus::Pen pen(Gdiplus::Color(200, 255, 0, 0), 3.0f); // Red with alpha 200
    pen.SetLineJoin(Gdiplus::LineJoinRound); // Rounded corners

    // Draw a rectangular frame (crosshair)
    int width = 100, height = 100; // Match the window size
    Gdiplus::Rect rect(0, 0, width - 1, height - 1);
    graphics.DrawRectangle(&pen, rect);
}

LRESULT CALLBACK GTACrossOver::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    GTACrossOver* pThis = nullptr;

    if (msg == WM_CREATE) {
        // Store the 'this' pointer in the window's user data
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
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void GTACrossOver::Run() {
    // Create the overlay window
    CreateOverlayWindow();
    if (!hwndOverlay) return;

    // Find the GTA 5 window
    FindGTAWindow();
    if (!hwndGTA) return;

    // Main message loop
    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0)) {
        // Update the overlay position dynamically
        UpdateOverlayPosition();

        // Redraw the window
        InvalidateRect(hwndOverlay, nullptr, TRUE);

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GTACrossOver app;
    app.Run();
    return 0;
}