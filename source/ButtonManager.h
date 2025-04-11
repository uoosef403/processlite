#ifndef ButtonManager_h
#define ButtonManager_h
#include <windows.h>

class ButtonManager {
public:
    explicit ButtonManager(const HWND hwndParent) : hwndParent(hwndParent), hwndButtonClose(nullptr) {}
    bool Create();

    static void OnCommand(WPARAM wParam);

private:
    HWND hwndParent;
    HWND hwndButtonClose;
};

#endif