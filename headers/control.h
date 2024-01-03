#ifndef MVM_CONTROL_H
#define MVM_CONTROL_H

#include "core.h"

/**
 * @brief Move the mouse to a certain position
 *
 * @param x the x position in pixels
 * @param y the y position in pixels
 */
void MoveTo(int x, int y) {
    SetCursorPos(x, y);
}

/**
 * @brief Do a single left mouse click
 *
 */
void Click(DWORD delay) {
    INPUT inputs[2];
    memset(inputs, 0, sizeof(INPUT) * 2);

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    SendInput(2, inputs, sizeof(INPUT));

    Sleep(delay);

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(2, inputs, sizeof(INPUT));
}

/**
 * @brief Do a single left mouse click at a certain position
 *
 * @param x the x position in pixels
 * @param y the y position in pixels
 */
void ClickAt(int x, int y) {
    SetCursorPos(x, y);

    INPUT inputs[2];
    memset(inputs, 0, sizeof(INPUT) * 2);

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(3, inputs, sizeof(INPUT));
}

#endif // !MVM_CONTROL_H
