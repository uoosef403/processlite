#include "SystemInfoPanel.h"
#include <commctrl.h>

constexpr int LABEL_WIDTH = 100;
constexpr int VALUE_WIDTH_MIN = 150;
constexpr int CONTROL_HEIGHT = 20;
constexpr int PADDING = 10;

bool SystemInfoPanel::Create(const int x, const int y, const int width, const int height) {
    if (!hwnd_parent_) return false;

    current_x_ = x;
    current_y_ = y;
    current_width_ = width;
    current_height_ = height;

    // --- Create Labels ---
    hwnd_os_label_ = CreateWindowW(WC_STATICW, L"OS Version:",
                               WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
                               0, 0, 10, 10, // Mock positions, we reposition later
                               hwnd_parent_, reinterpret_cast<HMENU>(IDC_STATIC_OS_LABEL), nullptr, nullptr);

    hwnd_cpu_label_ = CreateWindowW(WC_STATICW, L"CPU Info:",
                                WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
                                0, 0, 10, 10,
                                hwnd_parent_, reinterpret_cast<HMENU>(IDC_STATIC_CPU_LABEL), nullptr, nullptr);

    hwnd_ram_total_label_ = CreateWindowW(WC_STATICW, L"Total RAM:",
                                WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
                                0, 0, 10, 10,
                                hwnd_parent_, reinterpret_cast<HMENU>(IDC_STATIC_RAM_TOTAL_LABEL), nullptr, nullptr);

    hwnd_ram_avail_label_ = CreateWindowW(WC_STATICW, L"Available RAM:",
                                WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
                                0, 0, 10, 10,
                                hwnd_parent_, reinterpret_cast<HMENU>(IDC_STATIC_RAM_AVAIL_LABEL), nullptr, nullptr);


    // --- Create Value Fields (initially "N/A") ---
    hwnd_os_value_ = CreateWindowW(WC_STATICW, L"N/A",
                                WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
                                0, 0, 10, 10,
                                hwnd_parent_, reinterpret_cast<HMENU>(IDC_STATIC_OS_VALUE), nullptr, nullptr);

    hwnd_cpu_value_ = CreateWindowW(WC_STATICW, L"N/A",
                                 WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
                                 0, 0, 10, 10,
                                 hwnd_parent_, reinterpret_cast<HMENU>(IDC_STATIC_CPU_VALUE), nullptr, nullptr);

    hwnd_ram_total_value_ = CreateWindowW(WC_STATICW, L"N/A",
                                    WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
                                    0, 0, 10, 10,
                                    hwnd_parent_, reinterpret_cast<HMENU>(IDC_STATIC_RAM_TOTAL_VALUE), nullptr, nullptr);

    hwnd_ram_avail_value_ = CreateWindowW(WC_STATICW, L"N/A",
                                     WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
                                     0, 0, 10, 10,
                                     hwnd_parent_, reinterpret_cast<HMENU>(IDC_STATIC_RAM_AVAIL_VALUE), nullptr, nullptr);

    if (!hwnd_os_label_ || !hwnd_os_value_ || !hwnd_cpu_label_ || !hwnd_cpu_value_ ||
        !hwnd_ram_total_label_ || !hwnd_ram_total_value_ || !hwnd_ram_avail_label_ || !hwnd_ram_avail_value_)
    {
        return false;
    }

    // Apply initial layout
    RepositionControls();

    return true;
}

void SystemInfoPanel::RepositionControls() const {
    if (current_width_ < LABEL_WIDTH + VALUE_WIDTH_MIN + PADDING) return; // Not enough space

    int currentYPos = current_y_ + PADDING;
    const int labelX = current_x_ + PADDING;
    const int valueX = labelX + LABEL_WIDTH + PADDING;
    const int valueWidth = current_width_ - (LABEL_WIDTH + 3 * PADDING); // Calculate available width for value

    // OS Info
    if (hwnd_os_label_) MoveWindow(hwnd_os_label_, labelX, currentYPos, LABEL_WIDTH, CONTROL_HEIGHT, TRUE);
    if (hwnd_os_value_) MoveWindow(hwnd_os_value_, valueX, currentYPos, valueWidth, CONTROL_HEIGHT, TRUE);
    currentYPos += CONTROL_HEIGHT + PADDING;

    // CPU Info
    if (hwnd_cpu_label_) MoveWindow(hwnd_cpu_label_, labelX, currentYPos, LABEL_WIDTH, CONTROL_HEIGHT, TRUE);
    if (hwnd_cpu_value_) MoveWindow(hwnd_cpu_value_, valueX, currentYPos, valueWidth, CONTROL_HEIGHT, TRUE);
    currentYPos += CONTROL_HEIGHT + PADDING;

    // Total RAM
    if (hwnd_ram_total_label_) MoveWindow(hwnd_ram_total_label_, labelX, currentYPos, LABEL_WIDTH, CONTROL_HEIGHT, TRUE);
    if (hwnd_ram_total_value_) MoveWindow(hwnd_ram_total_value_, valueX, currentYPos, valueWidth, CONTROL_HEIGHT, TRUE);
    currentYPos += CONTROL_HEIGHT + PADDING;

    // Available RAM
    if (hwnd_ram_avail_label_) MoveWindow(hwnd_ram_avail_label_, labelX, currentYPos, LABEL_WIDTH, CONTROL_HEIGHT, TRUE);
    if (hwnd_ram_avail_value_) MoveWindow(hwnd_ram_avail_value_, valueX, currentYPos, valueWidth, CONTROL_HEIGHT, TRUE);
}

void SystemInfoPanel::Resize(const int parentWidth, const int parentHeight) {
    current_y_ = parentHeight - 160 - PADDING;
    current_width_ = parentWidth;
    current_height_ = parentHeight; // Store new dimensions
    RepositionControls(); // Recalculate layout
}

void SystemInfoPanel::SetOsVersion(const std::wstring& text) const {
    if (hwnd_os_value_) {
        SetWindowTextW(hwnd_os_value_, text.c_str());
    }
}

void SystemInfoPanel::SetCpuInfo(const std::wstring& text) const {
    if (hwnd_cpu_value_) {
        SetWindowTextW(hwnd_cpu_value_, text.c_str());
    }
}

void SystemInfoPanel::SetTotalRam(const std::wstring& text) const {
    if (hwnd_ram_total_value_) {
        SetWindowTextW(hwnd_ram_total_value_, text.c_str());
    }
}

void SystemInfoPanel::SetAvailableRam(const std::wstring& text) const {
    if (hwnd_ram_avail_value_) {
        SetWindowTextW(hwnd_ram_avail_value_, text.c_str());
    }
}
