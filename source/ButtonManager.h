#ifndef ButtonManager_h
#define ButtonManager_h
#include <windows.h>

class ButtonManager {
public:
    explicit ButtonManager(const HWND hwndParent) : hwndParent(hwndParent), hwndButtonClose(nullptr),
                                                    hwndButtonNew(nullptr),
                                                    hwndButtonKill(nullptr) {
    }

    bool Create();

    static void OnCommand(WPARAM wParam);

    void Resize(int parentWidth, int parentHeight) const;

private:
    HWND hwndParent;
    HWND hwndButtonClose;
    HWND hwndButtonNew;
    HWND hwndButtonKill;
};

#endif