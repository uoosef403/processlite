#include "ButtonManager.h"
#include <windows.h>
#include <commctrl.h>

bool ButtonManager::Create() {
    // Create two buttons on the left
    CreateWindowW(
        WC_BUTTON, L"New Process",
        WS_CHILD | WS_VISIBLE,
        10, 520, 100, 30, // Position and size
        hwndParent, reinterpret_cast<HMENU>(1), nullptr, nullptr
    );

    CreateWindowW(
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