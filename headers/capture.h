#ifndef MVM_CAPTURE_H
#define MVM_CAPTURE_H

#include "core.h"

// https://learn.microsoft.com/en-us/windows/win32/gdi/capturing-an-image
void CaptureSave(
    HDC hdc, HBITMAP hBmp,
    LPCSTR filename
) {
    BITMAP bmp;
    BITMAPINFOHEADER bmpInfo;
    BITMAPFILEHEADER bmpFile;

    HANDLE hFile = NULL;
    HANDLE hDIB = NULL;
    LPVOID lpBmp = NULL;
    DWORD dwBytesWritten = 0;
    DWORD dwSizeofDIB = 0;
    DWORD dwBmpSize = 0;

    memset(&bmp, 0, sizeof(BITMAP));
    memset(&bmpInfo, 0, sizeof(BITMAPINFOHEADER));
    memset(&bmpFile, 0, sizeof(BITMAPFILEHEADER));

    GetObject(hBmp, sizeof(BITMAP), &bmp);

    bmpInfo.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfo.biWidth = bmp.bmWidth;
    bmpInfo.biHeight = bmp.bmHeight;
    bmpInfo.biPlanes = 1;
    bmpInfo.biBitCount = 32;
    bmpInfo.biCompression = BI_RGB;
    bmpInfo.biSizeImage = 0;
    bmpInfo.biXPelsPerMeter = 0;
    bmpInfo.biYPelsPerMeter = 0;
    bmpInfo.biClrUsed = 0;
    bmpInfo.biClrImportant = 0;

    dwBmpSize = ((bmpInfo.biWidth * bmpInfo.biBitCount + 31) / 32) * 4 * bmpInfo.biHeight;

    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpBmp = GlobalLock(hDIB);

    GetDIBits(
        hdc, hBmp, 0, bmpInfo.biHeight, lpBmp,
        (BITMAPINFO *)&bmpInfo, DIB_RGB_COLORS
    );

    hFile = CreateFile(
        filename, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL
    );

    dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    bmpFile.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);
    bmpFile.bfSize = dwSizeofDIB;
    bmpFile.bfType = 0x4d42; // BM

    WriteFile(hFile, (LPSTR)&bmpFile, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bmpInfo, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpBmp, dwBmpSize, &dwBytesWritten, NULL);

    CloseHandle(hFile);

    GlobalUnlock(hDIB);
    GlobalFree(hDIB);
}

LONG CapturePixels(
    HBITMAP hBmp, uint32_t *pixels, DWORD numPixels
) {
    LONG numBits = GetBitmapBits(
        hBmp, sizeof(uint32_t) * numPixels, pixels
    );

    return numBits;
}

#endif // !MVM_CAPTURE_H
