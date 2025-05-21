#include "ButtonManager.h"
#include <windows.h>
#include <commctrl.h>

bool ButtonManager::Create() {
    // Create two buttons on the left
    hwnd_button_new_ = CreateWindowW(
        WC_BUTTON, L"New Process",
        WS_CHILD | WS_VISIBLE,
        10, 520, 100, 30, // Position and size
        hwnd_parent_, reinterpret_cast<HMENU>(1), nullptr, nullptr
    );

    hwnd_button_kill_ = CreateWindowW(
        WC_BUTTON, L"Kill Process",
        WS_CHILD | WS_VISIBLE,
        120, 520, 100, 30, // Position and size
        hwnd_parent_, reinterpret_cast<HMENU>(2), nullptr, nullptr
    );

    // Create a close button on the right
    hwnd_button_close_ = CreateWindowW(
        WC_BUTTON, L"Close",
        WS_CHILD | WS_VISIBLE,
        670, 520, 100, 30, // Position and size
        hwnd_parent_, reinterpret_cast<HMENU>(3), nullptr, nullptr
    );

    return hwnd_button_close_ != nullptr;
}

void ButtonManager::OnCommand(const WPARAM wParam) {
    if (LOWORD(wParam) == 3) { // Close button clicked
        PostQuitMessage(0);
    }
}

void ButtonManager::Resize(const int parentWidth, const int parentHeight) const {
    constexpr int padding = 10;
    constexpr int buttonWidth = 100;
    constexpr int buttonHeight = 30;
    const int y = parentHeight - buttonHeight - padding;

    // Position buttons on the bottom
    MoveWindow(hwnd_button_new_, padding, y, buttonWidth, buttonHeight, TRUE);
    MoveWindow(hwnd_button_kill_, padding + buttonWidth + padding, y, buttonWidth, buttonHeight, TRUE);
    MoveWindow(hwnd_button_close_, parentWidth - buttonWidth - padding, y, buttonWidth, buttonHeight, TRUE);
}
