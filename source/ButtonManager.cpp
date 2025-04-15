#include "ButtonManager.h"
#include <windows.h>
#include <commctrl.h>

bool ButtonManager::Create() {
    // Create two buttons on the left
    hwndButtonNew = CreateWindowW(
        WC_BUTTON, L"New Process",
        WS_CHILD | WS_VISIBLE,
        10, 520, 100, 30, // Position and size
        hwndParent, reinterpret_cast<HMENU>(1), nullptr, nullptr
    );

    hwndButtonKill = CreateWindowW(
        WC_BUTTON, L"Kill Process",
        WS_CHILD | WS_VISIBLE,
        120, 520, 100, 30, // Position and size
        hwndParent, reinterpret_cast<HMENU>(2), nullptr, nullptr
    );

    // Create a close button on the right
    hwndButtonClose = CreateWindowW(
        WC_BUTTON, L"Close",
        WS_CHILD | WS_VISIBLE,
        670, 520, 100, 30, // Position and size
        hwndParent, reinterpret_cast<HMENU>(3), nullptr, nullptr
    );

    return hwndButtonClose != nullptr;
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
    MoveWindow(hwndButtonNew, padding, y, buttonWidth, buttonHeight, TRUE);
    MoveWindow(hwndButtonKill, padding + buttonWidth + padding, y, buttonWidth, buttonHeight, TRUE);
    MoveWindow(hwndButtonClose, parentWidth - buttonWidth - padding, y, buttonWidth, buttonHeight, TRUE);
}
