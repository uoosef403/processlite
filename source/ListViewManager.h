#ifndef ListViewManager_h
#define ListViewManager_h
#include <map>
#include <windows.h>
#include <commctrl.h>

#include "ProcessInfo.h"

class ListViewManager {
public:
    explicit ListViewManager(const HWND hwndParent) : hwnd_parent_(hwndParent), hwnd_list_view_(nullptr) {}

    ~ListViewManager();

    bool Create();

    [[nodiscard]] HWND getHWND() const;

    void Resize(int parentWidth, int parentHeight) const;

    void applyListViewUpdates(ProcessUpdateData *updateData);

    int getOrAddIconIndex(const std::wstring &path);

private:
    HWND hwnd_parent_;
    HWND hwnd_list_view_;
    HIMAGELIST image_list_ = nullptr;
    std::map<std::wstring, int> icon_cache_;
};

#endif