#include "core.h"
#include "capture.h"
#include "control.h"

int main(int argc, char *argv[]) {
    HWND desktop = GetDesktopWindow();
    HWND console = GetConsoleWindow();
    HDC desktopDC = GetWindowDC(desktop);

    RECT desktopRect;
    GetWindowRect(desktop, &desktopRect);

    int captureX = desktopRect.left;
    int captureY = desktopRect.top;
    int captureW = desktopRect.right - desktopRect.left;
    int captureH = desktopRect.bottom - desktopRect.top;

    if (argc > 4) {
        captureX = atoi(argv[1]);
        captureY = atoi(argv[2]);
        captureW = atoi(argv[3]);
        captureH = atoi(argv[4]);
    }

    HDC captureDC = CreateCompatibleDC(desktopDC);
    HBITMAP captureBmp = CreateCompatibleBitmap(desktopDC, captureW, captureH);
    SelectObject(captureDC, captureBmp);

    BitBlt(
        captureDC, 0, 0, captureW, captureH,
        desktopDC, captureX, captureY,
        SRCCOPY
    );

    CaptureSave(captureDC, captureBmp, "capture.bmp");

    ReleaseDC(desktop, desktopDC);
    DeleteDC(captureDC);
    DeleteObject(captureBmp);

    return 0;
}
