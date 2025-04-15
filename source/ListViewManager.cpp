#include "ListViewManager.h"
#include <windows.h>
#include <commctrl.h>

bool ListViewManager::Create() {
    hwndListView = CreateWindowW(
        WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
        10, 10, 760, 500, // Position and size
        hwndParent, nullptr, nullptr, nullptr
    );

    if (!hwndListView) {
        return false;
    }

    // Initialize the list view
    LVCOLUMNW pNameClm = {};
    pNameClm.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    pNameClm.pszText = const_cast<wchar_t*>(L"Process Name");
    pNameClm.cx = 200;
    ListView_InsertColumn(hwndListView, 0, &pNameClm);

    LVCOLUMNW pPIDClm = {};
    pPIDClm.mask = LVCF_TEXT | LVCF_WIDTH;
    pPIDClm.pszText = const_cast<wchar_t*>(L"PID");
    pPIDClm.cx = 150;
    ListView_InsertColumn(hwndListView, 1, &pPIDClm);

    LVCOLUMNW pPath = {};
    pPath.mask = LVCF_TEXT | LVCF_WIDTH;
    pPath.pszText = const_cast<wchar_t*>(L"Path");
    pPath.cx = 200;
    ListView_InsertColumn(hwndListView, 2, &pPath);

    ListView_SetColumnWidth(hwndListView, 2, LVSCW_AUTOSIZE_USEHEADER);

    return true;
}

void ListViewManager::Resize(const int parentWidth, const int parentHeight) const {
    // Optional padding
    constexpr auto padding = 10;

    // Resize the list view to fit most of the window
    MoveWindow(hwndListView,
               padding,
               padding,
               parentWidth - 2 * padding,
               parentHeight - 6 * padding,
               TRUE);

    // Recalculate column widths (example: 30% + 20% + rest)
    const auto totalWidth = parentWidth - 2 * padding;

    const auto nameWidth = totalWidth * 30 / 100;
    const auto pidWidth  = totalWidth * 20 / 100;
    const auto pathWidth = totalWidth - nameWidth - pidWidth;

    ListView_SetColumnWidth(hwndListView, 0, nameWidth);
    ListView_SetColumnWidth(hwndListView, 1, pidWidth);
    ListView_SetColumnWidth(hwndListView, 2, pathWidth);
}
