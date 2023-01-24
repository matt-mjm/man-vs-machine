#include "core.h"
#include "capture.h"
#include "controll.h"

int main(int argc, char *argv[]) {
    if (argc < 7) {
        return 1;
    }

    HWND desktop = GetDesktopWindow();
    HWND console = GetConsoleWindow();
    HDC desktopDC = GetWindowDC(desktop);

    int captureX = atoi(argv[1]);
    int captureY = atoi(argv[2]);
    int captureW = atoi(argv[3]);
    int captureH = atoi(argv[4]);
    int stretchW = atoi(argv[5]);
    int stretchH = atoi(argv[6]);

    HDC captureDC = CreateCompatibleDC(desktopDC);
    HBITMAP captureBmp = CreateCompatibleBitmap(desktopDC, captureW, captureH);
    SelectObject(captureDC, captureBmp);

    HDC stretchDC = CreateCompatibleDC(desktopDC);
    HBITMAP stretchBmp = CreateCompatibleBitmap(desktopDC, stretchW, stretchH);
    SelectObject(stretchDC, stretchBmp);

    SetStretchBltMode(
        stretchDC, COLORONCOLOR
    );

    BitBlt(
        captureDC, 0, 0, captureW, captureH,
        desktopDC, captureX, captureY,
        SRCCOPY
    );

    StretchBlt(
        stretchDC, 0, 0, stretchW, stretchH,
        captureDC, 0, 0, captureW, captureH,
        SRCCOPY
    );

    CaptureSave(stretchDC, stretchBmp, "stretch.bmp");

    ReleaseDC(desktop, desktopDC);
    DeleteDC(captureDC);
    DeleteDC(stretchDC);
    DeleteObject(captureBmp);
    DeleteObject(stretchBmp);

    return 0;
}
