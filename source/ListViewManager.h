#ifndef ListViewManager_h
#define ListViewManager_h
#include <windows.h>

class ListViewManager {
public:
    explicit ListViewManager(const HWND hwndParent) : hwndParent(hwndParent), hwndListView(nullptr) {}

    bool Create();

    void Resize(int parentWidth, int parentHeight) const;

private:
    HWND hwndParent;
    HWND hwndListView;
};

#endif