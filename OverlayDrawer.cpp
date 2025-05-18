#include "OverlayDrawer.h"
#include <iostream>

void SetupConsole() {
    AllocConsole();
    FILE* dummy;
    freopen_s(&dummy, "CONOUT$", "w", stdout);
}

OverlayDrawer::OverlayDrawer() : hwndOverlay(nullptr), crosshairSize(50), lastRegion(nullptr) {
    SetupConsole();
    std::cout << "OverlayDrawer constructed" << std::endl;
}

OverlayDrawer::~OverlayDrawer() {
    if (lastRegion) DeleteObject(lastRegion);
    if (hwndOverlay) DestroyWindow(hwndOverlay);
}

void OverlayDrawer::CreateOverlayWindow(HINSTANCE hInstance) {
    std::cout << "Creating overlay window..." << std::endl;

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProcOverlay;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GTACrossOverOverlayWindow";
    wc.style = CS_HREDRAW | CS_VREDRAW;
    if (!RegisterClass(&wc)) {
        std::cout << "Failed to register overlay window class!" << std::endl;
        return;
    }

    hwndOverlay = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST,
        L"GTACrossOverOverlayWindow",
        L"GTA V Overlay",
        WS_POPUP,
        0, 0, crosshairSize, crosshairSize,
        nullptr, nullptr,
        hInstance,
        this
    );

    if (!hwndOverlay) {
        std::cout << "Failed to create overlay window!" << std::endl;
        return;
    }

    std::cout << "Overlay window created successfully: " << hwndOverlay << std::endl;

    COLORREF colorKey = RGB(255, 0, 255);
    SetLayeredWindowAttributes(hwndOverlay, colorKey, 0, LWA_COLORKEY);

    SetTimer(hwndOverlay, 2, 100, nullptr);
}

void OverlayDrawer::SetCrosshairSize(int size) {
    if (size < 50) size = 50;
    crosshairSize = size;
    std::cout << "Crosshair size set to: " << crosshairSize << std::endl;

    if (hwndOverlay) {
        SetWindowPos(hwndOverlay, nullptr, 0, 0, crosshairSize, crosshairSize,
            SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        InvalidateRect(hwndOverlay, nullptr, FALSE);
        UpdateWindow(hwndOverlay);
    }
}

void OverlayDrawer::UpdateWindowRegion(HWND hwndTarget) {
    if (!hwndTarget) {
        if (lastRegion) {
            DeleteObject(lastRegion);
            lastRegion = nullptr;
        }
        SetWindowRgn(hwndOverlay, nullptr, TRUE);
        return;
    }

    RECT targetRect;
    if (!GetWindowRect(hwndTarget, &targetRect)) {
        std::cout << "Failed to get target window rect for region" << std::endl;
        return;
    }

    RECT clientRect;
    if (!GetClientRect(hwndTarget, &clientRect)) {
        std::cout << "Failed to get client rect for region" << std::endl;
        return;
    }

    POINT clientTopLeft = { 0, 0 };
    ClientToScreen(hwndTarget, &clientTopLeft);
    RECT clientRectScreen = clientRect;
    OffsetRect(&clientRectScreen, clientTopLeft.x, clientTopLeft.y);

    HRGN newRegion = CreateRectRgn(
        clientRectScreen.left,
        clientRectScreen.top,
        clientRectScreen.right,
        clientRectScreen.bottom
    );

    POINT overlayPos = { 0, 0 };
    ClientToScreen(hwndOverlay, &overlayPos);
    OffsetRgn(newRegion, -overlayPos.x, -overlayPos.y);

    SetWindowRgn(hwndOverlay, newRegion, TRUE);

    if (lastRegion) DeleteObject(lastRegion);
    lastRegion = newRegion;
}

void OverlayDrawer::UpdateOverlayPosition(HWND hwndTarget, DisplayMode mode) {
    if (!hwndTarget) {
        std::cout << "No target window, hiding overlay" << std::endl;
        ShowWindow(hwndOverlay, SW_HIDE);
        UpdateWindowRegion(nullptr);
        return;
    }

    HWND foregroundWindow = GetForegroundWindow();
    if (foregroundWindow != hwndTarget) {
        std::cout << "GTA V is not the active window, hiding overlay" << std::endl;
        ShowWindow(hwndOverlay, SW_HIDE);
        UpdateWindowRegion(nullptr);
        return;
    }

    RECT targetRect;
    if (!GetWindowRect(hwndTarget, &targetRect)) {
        std::cout << "Failed to get target window rect, hiding overlay" << std::endl;
        ShowWindow(hwndOverlay, SW_HIDE);
        UpdateWindowRegion(nullptr);
        return;
    }

    int targetWidth = targetRect.right - targetRect.left;
    int targetHeight = targetRect.bottom - targetRect.top;

    int posX = targetRect.left + (targetWidth - crosshairSize) / 2;
    int posY = targetRect.top + (targetHeight - crosshairSize) / 2;

    if (mode == DisplayMode::Windowed) {
        // Hardcode the Y offset to 11 for Windowed mode
        const int yOffset = 11;
        posY += yOffset;
        std::cout << "Applied Y offset: " << yOffset << " pixels (Windowed mode)" << std::endl;
    }

    std::cout << "Updating overlay position to X: " << posX << ", Y: " << posY << std::endl;

    SetWindowPos(hwndOverlay, HWND_TOPMOST, posX, posY, crosshairSize, crosshairSize, SWP_SHOWWINDOW);
    UpdateWindowRegion(hwndTarget);
    InvalidateRect(hwndOverlay, nullptr, FALSE);
    UpdateWindow(hwndOverlay);
}

void OverlayDrawer::ShowOverlay() {
    std::cout << "Showing overlay window" << std::endl;
    ShowWindow(hwndOverlay, SW_SHOW);
    InvalidateRect(hwndOverlay, nullptr, FALSE);
    UpdateWindow(hwndOverlay);
}

void OverlayDrawer::HideOverlay() {
    std::cout << "Hiding overlay window" << std::endl;
    ShowWindow(hwndOverlay, SW_HIDE);
    UpdateWindowRegion(nullptr);
}

void OverlayDrawer::DrawDot(HDC hdc) {
    std::cout << "Drawing missile crosshair (GDI)" << std::endl;

    HBRUSH bgBrush = CreateSolidBrush(RGB(255, 0, 255));
    RECT bgRect = { 0, 0, crosshairSize, crosshairSize };
    FillRect(hdc, &bgRect, bgBrush);
    DeleteObject(bgBrush);

    int frameThickness = crosshairSize / 25;
    if (frameThickness < 2) frameThickness = 2;
    int padding = crosshairSize / 10;

    HPEN pen = CreatePen(PS_DASH, frameThickness, RGB(255, 255, 0));
    HPEN oldPen = (HPEN)SelectObject(hdc, pen);

    MoveToEx(hdc, padding, padding, nullptr);
    LineTo(hdc, crosshairSize - padding, padding);

    MoveToEx(hdc, crosshairSize - padding, padding, nullptr);
    LineTo(hdc, crosshairSize - padding, crosshairSize - padding);

    MoveToEx(hdc, crosshairSize - padding, crosshairSize - padding, nullptr);
    LineTo(hdc, padding, crosshairSize - padding);

    MoveToEx(hdc, padding, crosshairSize - padding, nullptr);
    LineTo(hdc, padding, padding);

    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

LRESULT CALLBACK OverlayDrawer::WndProcOverlay(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    OverlayDrawer* pThis = nullptr;

    if (msg == WM_CREATE) {
        pThis = reinterpret_cast<OverlayDrawer*>(((CREATESTRUCT*)lParam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    }
    else {
        pThis = reinterpret_cast<OverlayDrawer*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (pThis) {
        switch (msg) {
        case WM_TIMER:
            if (wParam == 2) {
                // Timer for position update is handled by GTACrossOver
            }
            return 0;
        case WM_PAINT: {
            std::cout << "WM_PAINT received, calling DrawDot" << std::endl;
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            pThis->DrawDot(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_ERASEBKGND:
            return 1;
        case WM_WINDOWPOSCHANGING:
            WINDOWPOS* wp = (WINDOWPOS*)lParam;
            wp->hwndInsertAfter = HWND_TOPMOST;
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}