#include "ListViewManager.h"
#include <windows.h>
#include <commctrl.h>

bool ListViewManager::Create() {
    hwndListView = CreateWindowW(
        WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL,
        10, 10, 780, 500, // Position and size
        hwndParent, nullptr, nullptr, nullptr
    );

    if (!hwndListView) {
        return false;
    }

    // Initialize the list view (e.g., add columns)
    LVCOLUMNW column = {};
    column.mask = LVCF_TEXT | LVCF_WIDTH;
    column.pszText = const_cast<wchar_t*>(L"Process Name");
    column.cx = 200;
    ListView_InsertColumn(hwndListView, 0, &column);

    return true;
}
