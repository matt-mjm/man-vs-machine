// https://www.mathsisfun.com/games/allout.html

#include "core.h"
#include "capture.h"
#include "control.h"

#define NUM_BITS (25)
#define NUM_STATES (1 << NUM_BITS)
#define QUEUE_SIZE (NUM_STATES)

uint8_t STATES[NUM_STATES];
uint32_t QUEUE[QUEUE_SIZE];
uint32_t QUEUE_HEAD = 0;
uint32_t QUEUE_TAIL = 0;

void ENQUEUE(uint32_t value) {
    QUEUE[QUEUE_TAIL++ % QUEUE_SIZE] = value;
}

uint32_t DEQUEUE(void) {
    return QUEUE[QUEUE_HEAD++ % QUEUE_SIZE];
}

uint32_t Step(uint32_t state, uint8_t move) {
    state ^= 1 << move;
    if (move % 5 > 0) state ^= 1 << (move - 1);
    if (move % 5 < 4) state ^= 1 << (move + 1);
    if (move / 5 > 0) state ^= 1 << (move - 5);
    if (move / 5 < 4) state ^= 1 << (move + 5);
    return state;
}

void Prepare(void) {
    for (uint32_t i = 0; i < NUM_STATES; i++) {
        STATES[i] = NUM_BITS;
    }

    ENQUEUE(0);

    while (QUEUE_HEAD < QUEUE_TAIL) {
        uint32_t state = DEQUEUE();

        for (uint8_t i = 0; i < NUM_BITS; i++) {
            uint32_t next = Step(state, i);
            if (STATES[next] == NUM_BITS) {
                STATES[next] = i;
                ENQUEUE(next);
            }
        }
    }
}

void Capture(uint32_t *pixels, int captureX, int captureY, int captureW, int captureH, int stretchW, int stretchH) {
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


    CapturePixels(stretchBmp, pixels, NUM_BITS);

    ReleaseDC(desktop, desktopDC);
    DeleteDC(captureDC);
    DeleteDC(stretchDC);
    DeleteObject(captureBmp);
    DeleteObject(stretchBmp);
}

int main(int argc, char *argv[]) {
    if (argc < 9) {
        return 1;
    }

    int captureX = atoi(argv[1]);
    int captureY = atoi(argv[2]);
    int captureW = atoi(argv[3]);
    int captureH = atoi(argv[4]);
    int stretchW = atoi(argv[5]);
    int stretchH = atoi(argv[6]);
    int nextX = atoi(argv[7]);
    int nextY = atoi(argv[8]);
    int tileW = captureW / stretchW;
    int tileH = captureH / stretchH;
    uint32_t pixels[NUM_BITS];

    Prepare();

    Capture(pixels, captureX, captureY, captureW, captureH, stretchW, stretchH);
    uint32_t dark = pixels[0];

    for (int iter = 0; iter < 102; iter++) {
        Capture(pixels, captureX, captureY, captureW, captureH, stretchW, stretchH);

        uint32_t state = 0;
        for (uint32_t i = 0; i < NUM_BITS; i++) {
            if (pixels[i] != dark) {
                state ^= 1 << i;
            }
        }

        while (state != 0) {
            uint8_t move = STATES[state];
            if (move < NUM_BITS) {
                state = Step(state, move);
            } else {
                printf("Error\n");
                break;
            }

            ClickAt(
                captureX + tileW / 2 + (move % 5) * tileW,
                captureY + tileH / 2 + (move / 5) * tileH
            );

            Sleep(10);
        }

        if (iter < 101) {
            ClickAt(nextX, nextY);
            Sleep(20);
        }
    }

    return 0;
}
