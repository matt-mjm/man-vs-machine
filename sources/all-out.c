// https://www.mathsisfun.com/games/allout.html

#include "core.h"
#include "capture.h"
#include "controll.h"

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

int main(int argc, char *argv[]) {
    int captureX = 1233;
    int captureY = 557;
    int captureW = 350;
    int captureH = 350;
    int stretchW = 5;
    int stretchH = 5;
    int tileW = captureW / stretchW;
    int tileH = captureH / stretchH;
    int nextX = 1500;
    int nextY = 500;

    Prepare();

    for (int iter = 0; iter < 102; iter++) {

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

    uint32_t pixels[NUM_BITS];
    CapturePixels(stretchBmp, pixels, NUM_BITS);

    ReleaseDC(desktop, desktopDC);
    DeleteDC(captureDC);
    DeleteDC(stretchDC);
    DeleteObject(captureBmp);
    DeleteObject(stretchBmp);

    uint32_t state = 0;
    for (uint32_t i = 0; i < NUM_BITS; i++) {
        if (pixels[i] == 0xFFCEF5CE) {
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
