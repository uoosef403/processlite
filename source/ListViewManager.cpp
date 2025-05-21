#include "ListViewManager.h"
#include <windows.h>
#include <commctrl.h>
#include <iostream>

#include "ProcessInfo.h"

ListViewManager::~ListViewManager() {
    if (image_list_) {
        ImageList_Destroy(image_list_);
    }
}

bool ListViewManager::Create() {
    hwnd_list_view_ = CreateWindowW(
        WC_LISTVIEW, L"",
        WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | WS_BORDER,
        10, 10, 760, 400, // Position and size
        hwnd_parent_, nullptr, nullptr, nullptr
    );

    if (!hwnd_list_view_) {
        return false;
    }

    // Initialize the list view
    LVCOLUMNW pNameClm = {};
    pNameClm.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_SUBITEM;
    pNameClm.pszText = const_cast<wchar_t *>(L"Process");
    pNameClm.cx = 200;
    ListView_InsertColumn(hwnd_list_view_, 0, &pNameClm);

    LVCOLUMNW pPIDClm = {};
    pPIDClm.mask = LVCF_TEXT | LVCF_WIDTH;
    pPIDClm.pszText = const_cast<wchar_t *>(L"PID");
    pPIDClm.cx = 50;
    ListView_InsertColumn(hwnd_list_view_, 1, &pPIDClm);

    LVCOLUMNW pCpu = {};
    pCpu.mask = LVCF_TEXT | LVCF_WIDTH;
    pCpu.pszText = const_cast<wchar_t *>(L"CPU");
    pCpu.cx = 50;
    ListView_InsertColumn(hwnd_list_view_, 2, &pCpu);

    LVCOLUMNW pRam = {};
    pRam.mask = LVCF_TEXT | LVCF_WIDTH;
    pRam.pszText = const_cast<wchar_t *>(L"Memory");
    pRam.cx = 50;
    ListView_InsertColumn(hwnd_list_view_, 3, &pRam);

    LVCOLUMNW pDisk = {};
    pDisk.mask = LVCF_TEXT | LVCF_WIDTH;
    pDisk.pszText = const_cast<wchar_t *>(L"Disk");
    pDisk.cx = 50;
    ListView_InsertColumn(hwnd_list_view_, 4, &pDisk);

    LVCOLUMNW pPath = {};
    pPath.mask = LVCF_TEXT | LVCF_WIDTH;
    pPath.pszText = const_cast<wchar_t *>(L"Path");
    pPath.cx = 150;
    ListView_InsertColumn(hwnd_list_view_, 5, &pPath);

    ListView_SetColumnWidth(hwnd_list_view_, 5, LVSCW_AUTOSIZE_USEHEADER);

    // Create and associate the ImageList
    image_list_ = ImageList_Create(
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        ILC_MASK | ILC_COLOR32,
        10, 100
    );
    if (!image_list_) {
        std::cerr << "Image list creation failed!" << std::endl;
    } else {
        ListView_SetImageList(hwnd_list_view_, image_list_, LVSIL_SMALL);
    }

    return true;
}

HWND ListViewManager::getHWND() const {
    return hwnd_list_view_;
}

void ListViewManager::Resize(const int parentWidth, const int parentHeight) const {
    // Optional padding
    constexpr auto padding = 10;

    // Resize the list view to fit most of the window
    MoveWindow(hwnd_list_view_,
               padding,
               padding,
               parentWidth - 2 * padding,
               parentHeight - 160 - 2 * padding,
               TRUE);

    // Recalculate column widths (example: 30% + 20% + rest)
    const auto totalWidth = parentWidth - 2 * padding;

    const auto nameWidth = totalWidth * 20 / 100;
    const auto pidWidth = totalWidth * 10 / 100;
    const auto cpuWidth = totalWidth * 10 / 100;
    const auto ramWidth = totalWidth * 10 / 100;
    const auto diskWidth = totalWidth * 10 / 100;
    const auto pathWidth = totalWidth - nameWidth - cpuWidth - ramWidth - diskWidth - pidWidth;

    ListView_SetColumnWidth(hwnd_list_view_, 0, nameWidth);
    ListView_SetColumnWidth(hwnd_list_view_, 1, pidWidth);
    ListView_SetColumnWidth(hwnd_list_view_, 2, cpuWidth);
    ListView_SetColumnWidth(hwnd_list_view_, 3, ramWidth);
    ListView_SetColumnWidth(hwnd_list_view_, 4, diskWidth);
    ListView_SetColumnWidth(hwnd_list_view_, 5, pathWidth);
}

void ListViewManager::applyListViewUpdates(ProcessUpdateData* updateData) {
    if (!updateData) return;

    // Improve performance by temporarily disabling redraw
    SendMessageW(hwnd_list_view_, WM_SETREDRAW, FALSE, 0);

    // Get current item count for finding items
    unsigned long itemCount = ListView_GetItemCount(hwnd_list_view_);
    std::vector<DWORD> pidsInList(itemCount);
    for (int i = 0; i < itemCount; ++i) {
        LVITEMW lvi;
        lvi.mask = LVIF_PARAM;
        lvi.iItem = i;
        if (ListView_GetItem(hwnd_list_view_, &lvi)) {
            pidsInList[i] = static_cast<DWORD>(lvi.lParam);
        } else {
            pidsInList[i] = 0; // Mark as invalid if GetItem fails
        }
    }


    // 1. Remove items for processes that have exited
    for (DWORD removedPid: updateData->removed_pids) {
        for (int i = 0; i < itemCount; ++i) {
            if (pidsInList[i] == removedPid) {
                ListView_DeleteItem(hwnd_list_view_, i);
                itemCount--;
                pidsInList.erase(pidsInList.begin() + i);
                break;
            }
        }
    }

    // 2. Update existing items
    for (const auto &updatedInfo: updateData->updated) {
        bool found = false;
        for (int i = 0; i < itemCount; ++i) {
            if (pidsInList[i] == updatedInfo.pid) {
                LVITEMW item = {0};
                item.iItem = i;
                item.mask = LVIF_TEXT | LVIF_IMAGE;

                int iconIdx = getOrAddIconIndex(updatedInfo.path);
                item.iImage = (iconIdx != -1) ? iconIdx : I_IMAGENONE;

                // Update Name (SubItem 0)
                item.iSubItem = 0;
                item.pszText = const_cast<wchar_t *>(updatedInfo.name.c_str());
                ListView_SetItem(hwnd_list_view_, &item);

                // Update other subitems using ListView_SetItemText for efficiency
                wchar_t buffer[64]; // Buffer for formatted strings

                // SubItem 1: PID (Usually doesn't change, but update for consistency)
                swprintf_s(buffer, L"%lu", updatedInfo.pid);
                ListView_SetItemText(hwnd_list_view_, i, 1, buffer);

                // SubItem 2: CPU Usage
                swprintf_s(buffer, L"%.1f %%", updatedInfo.cpuUsage); // Format CPU
                ListView_SetItemText(hwnd_list_view_, i, 2, buffer);

                // SubItem 3: RAM Usage (convert bytes to KB or MB)
                swprintf_s(buffer, L"%zu KB", updatedInfo.ramUsage / 1024);
                ListView_SetItemText(hwnd_list_view_, i, 3, buffer);

                // SubItem 4: I/O Rate (convert B/s to KB/s or MB/s)
                swprintf_s(buffer, L"%.1f KB/s", updatedInfo.ioRate / 1024.0);
                ListView_SetItemText(hwnd_list_view_, i, 4, buffer);

                // SubItem 5: Path (maybe truncate)
                std::wstring displayPath = updatedInfo.path;
                if (constexpr size_t maxPathLen = 40; displayPath.length() > maxPathLen) {
                    displayPath = L"..." + displayPath.substr(displayPath.length() - maxPathLen);
                }
                ListView_SetItemText(hwnd_list_view_, i, 5, const_cast<wchar_t*>(displayPath.c_str()));

                found = true;
                break; // Move to next updated PID
            }
        }
        if (!found) {
            std::cerr << "UI Update: PID " << updatedInfo.pid << " marked for update but not found in list.\n";
        }
    }

    // 3. Add new items
    for (const auto &addedInfo: updateData->added) {
        LVITEMW item = {0};
        item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE; // Include LVIF_IMAGE
        // Insert at the end for simplicity, or implement sorting
        item.iItem = ListView_GetItemCount(hwnd_list_view_);
        item.lParam = static_cast<LPARAM>(addedInfo.pid); // Store PID directly!
        item.pszText = const_cast<wchar_t *>(addedInfo.name.c_str());

        // Get/Add Icon
        int iconIdx = getOrAddIconIndex(addedInfo.path);
        item.iImage = (iconIdx != -1) ? iconIdx : I_IMAGENONE;

        int newItemIndex = ListView_InsertItem(hwnd_list_view_, &item);
        if (newItemIndex != -1) {
            wchar_t buffer[64];

            // Set subitems for the newly added item
            swprintf_s(buffer, L"%lu", addedInfo.pid);
            ListView_SetItemText(hwnd_list_view_, newItemIndex, 1, buffer);

            swprintf_s(buffer, L"%.1f %%", addedInfo.cpuUsage);
            ListView_SetItemText(hwnd_list_view_, newItemIndex, 2, buffer);

            swprintf_s(buffer, L"%llu KB", addedInfo.ramUsage / 1024);
            ListView_SetItemText(hwnd_list_view_, newItemIndex, 3, buffer);

            swprintf_s(buffer, L"%.1f KB/s", addedInfo.ioRate / 1024.0);
            ListView_SetItemText(hwnd_list_view_, newItemIndex, 4, buffer);

            ListView_SetItemText(hwnd_list_view_, newItemIndex, 5, const_cast<wchar_t*>(addedInfo.path.c_str()));
        } else {
            std::cerr << "UI Update: Failed to insert item for PID " << addedInfo.pid << std::endl;
        }
    }

    // Re-enable redraw
    SendMessageW(hwnd_list_view_, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(hwnd_list_view_, nullptr, TRUE);
    delete updateData;
}

int ListViewManager::getOrAddIconIndex(const std::wstring& path) {
     if (path.empty() || !image_list_) {
         return -1;
     }

     if (const auto cacheIt = icon_cache_.find(path); cacheIt != icon_cache_.end()) {
        return cacheIt->second; // Found in cache
    }

    // Not in cache, try to extract icon
    HICON hIcon = nullptr;
    SHFILEINFOW sfi = {nullptr};
     if (SHGetFileInfoW(path.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON)) {
         hIcon = sfi.hIcon;
     } else {
         ExtractIconExW(path.c_str(), 0, nullptr, &hIcon, 1);
     }


    if (hIcon) {
        if (const int index = ImageList_ReplaceIcon(image_list_, -1, hIcon); index != -1) {
            icon_cache_[path] = index; // Add to cache
            return index;
        }
        if (!SHGetFileInfoW(path.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON)) {
            DestroyIcon(hIcon);
        }
        return -1;
    }

    return -1;
}
