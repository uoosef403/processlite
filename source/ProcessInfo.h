#ifndef ProcessInfo_h
#define ProcessInfo_h

#include <chrono>
#include <Windows.h>
#include <string>

struct ProcessInfo {
    DWORD pid = 0;
    std::wstring name;
    std::wstring path;
    double cpuUsage = 0.0;
    SIZE_T ramUsage = 0;
    double ioRate = 0.0;
    int iconIndex = -1;
    std::chrono::steady_clock::time_point lastUpdateTime;

    ProcessInfo() = default;

    explicit ProcessInfo(const DWORD id) : pid(id), lastUpdateTime(std::chrono::steady_clock::now()){}
};

#include <vector>
struct ProcessUpdateData {
    std::vector<ProcessInfo> added;
    std::vector<DWORD> removed_pids;
    std::vector<ProcessInfo> updated;
};

#endif