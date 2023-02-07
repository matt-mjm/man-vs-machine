// https://webgamesonline.com/samegame/index.php

#include "core.h"
#include "capture.h"
#include "controll.h"

#define BOARD_W 16
#define BOARD_H 12

uint8_t LoadBoard(uint8_t *board, uint32_t *pixels) {
    uint8_t value = 1;

    for (int i = 0; i < BOARD_W * BOARD_H; i++) {
        uint32_t colour = pixels[i];
        bool duplicate = false;

        for (int j = 0; j < i; j++) {
            if (pixels[j] == colour) {
                duplicate = true;
                break;
            }
        }

        if (duplicate) {
            continue;
        }

        for (int j = 0; j < BOARD_W * BOARD_H; j++) {
            if (pixels[j] == colour) {
                board[j] = value;
            }
        }

        value += 1;
    }

    return value;
}

void DisplayBoard(uint8_t *board) {
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            SetConsoleTextAttribute(console, board[x + y * BOARD_W] * 16);
            printf("  ");
        }
        SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
        printf("\n");
    }
}

void CopyBoard(
    uint8_t *dst, const uint8_t *src
) {
    memcpy(dst, src, sizeof(uint8_t) * BOARD_W * BOARD_H);
}

uint8_t FloodBoard(
    uint8_t *board,
    uint8_t colour,
    int x, int y
) {
    if (colour == 0) {
        return 0;
    }
    if (x < 0 || y < 0 || x >= BOARD_W || y >= BOARD_H) {
        return 0;
    }
    if (board[x + y * BOARD_W] != colour) {
        return 0;
    }

    uint8_t count = 1;
    board[x + y * BOARD_W] = 0;
    count += FloodBoard(board, colour, x - 1, y);
    count += FloodBoard(board, colour, x, y - 1);
    count += FloodBoard(board, colour, x + 1, y);
    count += FloodBoard(board, colour, x, y + 1);
    return count;
}

void UpdateBoard(
    uint8_t *board
) {
    int xDiff = 0;
    for (int x = 0; x < BOARD_W; x++) {
        int yDiff = 0;
        for (int y = BOARD_H - 1; y >= 0; y--) {
            if (board[x + y * BOARD_W] == 0) {
                yDiff += 1;
            } else if (xDiff != 0 || yDiff != 0) {
                board[(x + xDiff) + (y + yDiff) * BOARD_W] = board[x + y * BOARD_W];
                board[x + y * BOARD_W] = 0;
            }
        }
        if (yDiff >= BOARD_H) {
            xDiff -= 1;
        }
    }
}

uint8_t CalculateMoves(uint8_t *board, uint8_t *moves) {
    static uint8_t local[BOARD_W * BOARD_H];
    CopyBoard(local, board);

    uint8_t numMoves = 0;
    for (uint8_t i = 0; i < BOARD_W * BOARD_H; i++) {
        uint16_t count = FloodBoard(local, board[i], i % BOARD_W, i / BOARD_W);
        if (count < 2) {
            continue;
        }

        moves[numMoves] = i;
        numMoves += 1;
    }

    return numMoves;
}

uint8_t SolveBoard(
    const uint8_t *board, uint8_t *moves, uint16_t *totalScore
) {
    uint8_t localBoard[BOARD_W * BOARD_H];
    uint8_t localMoves[BOARD_W * BOARD_H];
    uint16_t numMoves = 0;
    uint16_t localScore = 0;

    CopyBoard(localBoard, board);

    for (;;) {
        uint8_t validMoves = CalculateMoves(localBoard, localMoves);
        if (validMoves == 0) {
            break;
        }

        uint8_t move = localMoves[rand() % validMoves];
        moves[numMoves] = move;
        numMoves += 1;

        uint16_t count = FloodBoard(localBoard, localBoard[move], move % BOARD_W, move / BOARD_W);
        UpdateBoard(localBoard);

        localScore += (count - 2) * (count - 2);
        // localScore += count;
    }

    *totalScore = localScore;
    return numMoves;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    if (argc < 10) {
        return 1;
    }

    int captureX = atoi(argv[1]);
    int captureY = atoi(argv[2]);
    int captureW = atoi(argv[3]);
    int captureH = atoi(argv[4]);
    int stretchW = atoi(argv[5]);
    int stretchH = atoi(argv[6]);
    int restartX = atoi(argv[7]);
    int restartY = atoi(argv[8]);
    int targetScore = atoi(argv[9]);

    if (stretchW != BOARD_W || stretchH != BOARD_H) {
        return 1;
    }

capture_start:

    HWND desktop = GetDesktopWindow();
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

    uint32_t pixels[BOARD_W * BOARD_H];
    CapturePixels(stretchBmp, pixels, BOARD_W * BOARD_H);

    ReleaseDC(desktop, desktopDC);
    DeleteDC(captureDC);
    DeleteDC(stretchDC);
    DeleteObject(captureBmp);
    DeleteObject(stretchBmp);

    uint8_t board[BOARD_W * BOARD_H];
    LoadBoard(board, pixels);

    int retry_count = 0;
solution_start:

    uint16_t totalScore = 0;
    uint8_t moves[BOARD_W * BOARD_H];
    uint16_t numMoves = SolveBoard(board, moves, &totalScore);

    static uint16_t maxScore = 0;
    if (maxScore < totalScore) {
        maxScore = totalScore;
        printf("Total Score = %7d / %7d\r", maxScore, targetScore);
        fflush(stdout);
    }

    if (totalScore < targetScore) {
        if (retry_count++ < 10000) {
            goto solution_start;
        } else {
            ClickAt(restartX, restartY);
            Sleep(500);

            goto capture_start;
        }
    }

    for (uint8_t i = 0; i < numMoves; i++) {
        MoveTo(
            captureX + (captureW / stretchW) / 2 + (moves[i] % BOARD_W) * (captureW / stretchW),
            captureY + (captureH / stretchH) / 2 + (moves[i] / BOARD_W) * (captureH / stretchH)
        );
        Sleep(250);
        Click(125);
        Sleep(125);
    }

    printf("Total Score = %7d                \n", totalScore);

    return 0;
}
