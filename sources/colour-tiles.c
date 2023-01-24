#include "core.h"
#include "capture.h"
#include "controll.h"

typedef struct boardinfo_s boardinfo_t;
struct boardinfo_s {
    int boardX;
    int boardY;
    int tileW;
    int tileH;
};

typedef struct tile_s tile_t;
struct tile_s {
    uint32_t colour;
    int D[4];
};

int DELTA[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };

void ClickBoard(boardinfo_t *boardinfo, int x, int y) {
    ClickAt(
        boardinfo->boardX + boardinfo->tileW / 2 + boardinfo->tileW * x,
        boardinfo->boardY + boardinfo->tileH / 2 + boardinfo->tileH * y
    );
    Sleep(15);
}

void UpdateTile(tile_t *tiles, tile_t *tile, int x, int y, int width, int height) {
    if (tile->colour != 0) {
        tile->D[0] = 0;
        tile->D[1] = 0;
        tile->D[2] = 0;
        tile->D[3] = 0;
        return;
    }

    uint32_t colours[4] = { 0, 0, 0, 0 };
    for (int dir = 0; dir < 4; dir++) {
        int t = 1;
        int dx = DELTA[dir][0];
        int dy = DELTA[dir][1];

        while (
            x + t * dx >= 0 && x + t * dx < width &&
            y + t * dy >= 0 && y + t * dy < height &&
            tiles[(x + t * dx) + (y + t * dy) * width].colour == 0
        ) {
            t += 1;
        }

        if (x + t * dx >= 0 && x + t * dx < width &&
            y + t * dy >= 0 && y + t * dy < height
        ) {
            tile->D[dir] = t;
            colours[dir] = tiles[(x + t * dx) + (y + t * dy) * width].colour;
        } else {
            tile->D[dir] = 0;
            colours[dir] = 0;
        }
    }

    for (int i = 0; i < 4; i++) {
        bool isValid = false;
        for (int j = 0; j < 4; j++) {
            if (i != j && colours[i] == colours[j] && colours[i] != 0) {
                isValid = true;
                break;
            }
        }
        if (isValid == false) {
            tile->D[i] = 0;
        }
    }
}

void PlayGame(boardinfo_t *boardinfo, tile_t *tiles, int width, int height) {
    int numMoves = 1;
    int *moves = (int *)malloc(sizeof(int) * width * height);
    if (moves == NULL) {
        return;
    }

    while (numMoves > 0) {
        numMoves = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                tile_t *tile = &tiles[x + y * width];
                UpdateTile(tiles, tile, x, y, width, height);

                int value = (tile->D[0] != 0) +
                    (tile->D[1] != 0) +
                    (tile->D[2] != 0) +
                    (tile->D[3] != 0);

                if (value == 2 || value == 4) {
                    moves[numMoves] = x + y * width;
                    numMoves += 1;
                }
            }
        }

        if (numMoves > 0) {
            int moveIndex = moves[rand() % numMoves];
            int x = moveIndex % width;
            int y = moveIndex / width;
            tile_t *tile = &tiles[moveIndex];

            ClickBoard(boardinfo, x, y);

            tiles[(x - tile->D[0]) + y * width].colour = 0;
            tiles[x + (y - tile->D[1]) * width].colour = 0;
            tiles[(x + tile->D[2]) + y * width].colour = 0;
            tiles[x + (y + tile->D[3]) * width].colour = 0;
        }
    }

    free(moves);
}

int main(int argc, char *argv[]) {
    if (argc < 7) {
        return 1;
    }

    int captureX = atoi(argv[1]);
    int captureY = atoi(argv[2]);
    int captureW = atoi(argv[3]);
    int captureH = atoi(argv[4]);
    int stretchW = atoi(argv[5]);
    int stretchH = atoi(argv[6]);

    tile_t *tiles = (tile_t *)malloc(sizeof(tile_t) * stretchW * stretchH);
    if (tiles == NULL) {
        return 1;
    }

    boardinfo_t boardinfo = { captureX, captureY, captureW / stretchW, captureH / stretchH };

    HWND desktop = GetDesktopWindow();
    HWND console = GetConsoleWindow();
    HDC desktopDC = GetWindowDC(desktop);

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

    uint32_t *pixels = CapturePixels(stretchBmp);

    ReleaseDC(desktop, desktopDC);
    DeleteDC(captureDC);
    DeleteDC(stretchDC);
    DeleteObject(captureBmp);
    DeleteObject(stretchBmp);

    if (pixels == NULL) {
        return 1;
    }

    for (int i = 0; i < stretchW * stretchH; i++) {
        if (pixels[i] == 0xffebeded || pixels[i] == 0xfff5f7f7) {
            tiles[i].colour = 0;
        } else {
            tiles[i].colour = pixels[i];
        }
    }

    free(pixels);

    PlayGame(&boardinfo, tiles, stretchW, stretchH);

    free(tiles);

    return 0;
}
