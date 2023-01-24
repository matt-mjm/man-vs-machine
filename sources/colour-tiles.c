#include "core.h"
#include "capture.h"
#include "controll.h"

#define BOARD_W 23
#define BOARD_H 15

#define TILE_W 25
#define TILE_H 25

int IMG_X = 0;
int IMG_Y = 0;
#define IMG_W (BOARD_W * TILE_W)
#define IMG_H (BOARD_H * TILE_H)

typedef struct tile_s tile_t;
struct tile_s {
    int tile;
    int value;
    int D[4];
};

void ClickBoard(int x, int y) {
    ClickAt(
        IMG_X + TILE_W / 2 + TILE_W * x,
        IMG_Y + TILE_H / 2 + TILE_H * y
    );
}

bool IsWhite(uint32_t colour) {
    uint8_t *pixels = (uint8_t *)&colour;
    return pixels[0] >= 0xE0 &&
        pixels[1] >= 0xE0 && pixels[2] >= 0xE0;
}

bool ColourMatch(uint32_t colour0, uint32_t colour1) {
    uint8_t *pixels0 = (uint8_t *)&colour0;
    uint8_t *pixels1 = (uint8_t *)&colour1;

    int diff = 0;
    for (int i = 0; i < 4; i++) {
        int d = (int)pixels0[i] - (int)pixels1[i];
        diff += d < 0 ? -d : d;
    }

    return diff < 16;
}

uint32_t BMP_BOARD[BOARD_W * BOARD_H];
uint32_t COLOURS[BOARD_W * BOARD_H];
int DIR_DELTA[4][2] = { { -1, 0 }, { 0, -1 }, { 1, 0 }, { 0, 1 } };

tile_t TILES[BOARD_W][BOARD_H];

void LoadTiles(HBITMAP hBmp) {
    LONG board_len = GetBitmapBits(
        hBmp, sizeof(BMP_BOARD),
        BMP_BOARD
    );

    uint8_t numColours = 0;
    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            uint8_t value = 0;
            uint32_t colour = BMP_BOARD[x + y * BOARD_W];

            if (IsWhite(colour)) {
                continue;
            }

            for (int i = 0; i < numColours; i++) {
                if (!ColourMatch(colour, COLOURS[i])) {
                    continue;
                }

                value = i + 1;
                break;
            }

            if (value == 0) {
                COLOURS[numColours] = colour;
                numColours += 1;
                value = numColours;
            }

            TILES[x][y].tile = value;
        }
    }
}

void UpdateTileDir(int x, int y, int dir) {
    tile_t *tile = &TILES[x][y];

    int t = 1;
    int dx = DIR_DELTA[dir][0];
    int dy = DIR_DELTA[dir][1];

    while (
        x + t * dx >= 0 && x + t * dx < BOARD_W &&
        y + t * dy >= 0 && y + t * dy < BOARD_H &&
        TILES[x + t * dx][y + t * dy].tile == 0
    ) {
        t += 1;
    }

    if (x + t * dx >= 0 && x + t * dx < BOARD_W &&
        y + t * dy >= 0 && y + t * dy < BOARD_H
    ) {
        tile->D[dir] = t;
    } else {
        tile->D[dir] = 0;
    }
}

void UpdateTile(int x, int y) {
    tile_t *tile = &TILES[x][y];

    if (tile->tile == 0) {
        UpdateTileDir(x, y, 0);
        UpdateTileDir(x, y, 1);
        UpdateTileDir(x, y, 2);
        UpdateTileDir(x, y, 3);
    } else {
        tile->D[0] = 0;
        tile->D[1] = 0;
        tile->D[2] = 0;
        tile->D[3] = 0;
    }

    int colours[4] = { 0, 0, 0, 0 };
    colours[0] = TILES[x - tile->D[0]][y].tile;
    colours[1] = TILES[x][y - tile->D[1]].tile;
    colours[2] = TILES[x + tile->D[2]][y].tile;
    colours[3] = TILES[x][y + tile->D[3]].tile;

    for (int i = 0; i < 4; i++) {
        bool isValid = false;
        for (int j = 0; j < 4; j++) {
            if (i != j && colours[i] == colours[j] && colours[i] != 0) {
                isValid = true;
                break;
            }
        }
        if (!isValid) {
            tile->D[i] = 0;
        }
    }

    tile->value = 0;
    for (int i = 0; i < 4; i++) {
        tile->value += tile->D[i] != 0 ? 1 : 0;
    }
}

void UpdateTiles(void) {
    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            UpdateTile(x, y);
        }
    }
}

bool DoNextMove(void) {
    UpdateTiles();

    int numMoves = 0;
    int MOVES[BOARD_W * BOARD_H][2];

    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            if (TILES[x][y].value == 2 || TILES[x][y].value == 4) {
                MOVES[numMoves][0] = x;
                MOVES[numMoves][1] = y;
                numMoves += 1;
            }
        }
    }

    if (numMoves > 0) {
        int move = rand() % numMoves;
        int x = MOVES[move][0];
        int y = MOVES[move][1];
        tile_t *tile = &TILES[x][y];

        ClickBoard(x, y);

        TILES[x - tile->D[0]][y].tile = 0;
        TILES[x][y - tile->D[1]].tile = 0;
        TILES[x + tile->D[2]][y].tile = 0;
        TILES[x][y + tile->D[3]].tile = 0;

        return true;
    }

    return false;
}

void PlayGame(void) {
    int count = 0;
    while (DoNextMove() && count < 100) {
        Sleep(100);
        count = count + 1;
    }
}

int main(int argc, char *argv[]) {
    HWND hwnd = GetDesktopWindow();
    RECT rect[1] = { 0 };
    HDC hdc[4] = { NULL, NULL, NULL, NULL };
    HBITMAP hBmp[4] = { NULL, NULL, NULL, NULL };

    // ShowWindow(GetConsoleWindow(), SW_MINIMIZE);
    // Sleep(1000);

    GetWindowRect(hwnd, rect);

    int WIN_X = rect[0].left;
    int WIN_Y = rect[0].top;
    int WIN_W = rect[0].right - rect[0].left;
    int WIN_H = rect[0].bottom - rect[0].top;

    IMG_X = argc > 1 ? atoi(argv[1]) : 0;
    IMG_Y = argc > 2 ? atoi(argv[2]) : 0;

    hdc[0] = GetWindowDC(hwnd);

    hBmp[1] = CreateCompatibleBitmap(hdc[0], WIN_W, WIN_H);
    hdc[1] = CreateCompatibleDC(hdc[0]);
    SelectObject(hdc[1], hBmp[1]);

    hBmp[2] = CreateCompatibleBitmap(hdc[0], IMG_W, IMG_H);
    hdc[2] = CreateCompatibleDC(hdc[0]);
    SelectObject(hdc[2], hBmp[2]);

    hBmp[3] = CreateCompatibleBitmap(hdc[0], BOARD_W, BOARD_H);
    hdc[3] = CreateCompatibleDC(hdc[0]);
    SelectObject(hdc[3], hBmp[3]);

    BitBlt(
        hdc[1], 0, 0, WIN_W, WIN_H,
        hdc[0], WIN_X, WIN_Y,
        SRCCOPY
    );

    BitBlt(
        hdc[2], 0, 0, IMG_W, IMG_H,
        hdc[1], IMG_X, IMG_Y,
        SRCCOPY
    );

    StretchBlt(
        hdc[3], 0, 0, BOARD_W, BOARD_H,
        hdc[2], TILE_W / 2, TILE_H / 2, IMG_W, IMG_H,
        SRCCOPY
    );

    if (argc > 3) {
        LoadTiles(hBmp[3]);
        PlayGame();
    }

    SaveHBITMAPToFile(hBmp[1], "image-1.bmp");
    SaveHBITMAPToFile(hBmp[2], "image-2.bmp");
    SaveHBITMAPToFile(hBmp[3], "image-3.bmp");

    ReleaseDC(hwnd, hdc[0]);
    DeleteDC(hdc[1]);
    DeleteDC(hdc[2]);
    DeleteDC(hdc[3]);

    DeleteObject(hBmp[1]);
    DeleteObject(hBmp[2]);
    DeleteObject(hBmp[3]);

    return 0;
}
