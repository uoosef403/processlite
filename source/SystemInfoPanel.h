#ifndef SystemInfoPanel_h
#define SystemInfoPanel_h

#include <Windows.h>
#include <string>

constexpr int IDC_STATIC_OS_LABEL = 100;
constexpr int IDC_STATIC_OS_VALUE = 102;
constexpr int IDC_STATIC_CPU_LABEL = 103;
constexpr int IDC_STATIC_CPU_VALUE = 104;
constexpr int IDC_STATIC_RAM_TOTAL_LABEL = 105;
constexpr int IDC_STATIC_RAM_TOTAL_VALUE = 106;
constexpr int IDC_STATIC_RAM_AVAIL_LABEL = 107;
constexpr int IDC_STATIC_RAM_AVAIL_VALUE = 108;

class SystemInfoPanel {
public:
    explicit SystemInfoPanel(const HWND parent) : hwnd_parent_(parent) {};
    ~SystemInfoPanel() = default;

    SystemInfoPanel(const SystemInfoPanel&) = delete;
    SystemInfoPanel& operator=(const SystemInfoPanel&) = delete;
    SystemInfoPanel(SystemInfoPanel&&) = delete;
    SystemInfoPanel& operator=(SystemInfoPanel&&) = delete;

    bool Create(int x, int y , int width, int height);
    void Resize(int parentWidth, int parentHeight);
    void RepositionControls() const;

    void SetOsVersion(const std::wstring& text) const;
    void SetCpuInfo(const std::wstring& text) const;
    void SetTotalRam(const std::wstring& text) const;
    void SetAvailableRam(const std::wstring& text) const;

private:
    HWND hwnd_parent_ = nullptr;

    HWND hwnd_os_label_ = nullptr;
    HWND hwnd_os_value_ = nullptr;
    HWND hwnd_cpu_label_ = nullptr;
    HWND hwnd_cpu_value_ = nullptr;
    HWND hwnd_ram_total_label_ = nullptr;
    HWND hwnd_ram_total_value_ = nullptr;
    HWND hwnd_ram_avail_label_ = nullptr;
    HWND hwnd_ram_avail_value_ = nullptr;

    int current_x_ = 0;
    int current_y_ = 0;
    int current_width_ = 0;
    int current_height_ = 0;
};
#endif