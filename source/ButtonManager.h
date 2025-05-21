#ifndef ButtonManager_h
#define ButtonManager_h
#include <windows.h>

class ButtonManager {
public:
    explicit ButtonManager(const HWND hwndParent)
        : hwnd_parent_(hwndParent), hwnd_button_close_(nullptr),
          hwnd_button_new_(nullptr),
          hwnd_button_kill_(nullptr) {
    }

    bool Create();

    static void OnCommand(WPARAM wParam);

    void Resize(int parentWidth, int parentHeight) const;

private:
    HWND hwnd_parent_;
    HWND hwnd_button_close_;
    HWND hwnd_button_new_;
    HWND hwnd_button_kill_;
};

#endif
