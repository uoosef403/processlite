#include "ProcessMetrics.h"
#include <psapi.h>
#include <chrono>
#include <iostream>
#include <ranges>

using namespace std::chrono;


uint64_t ProcessMetrics::FileTimeToUint(const FILETIME& ft)
{
    const ULARGE_INTEGER ui{ ft.dwLowDateTime, ft.dwHighDateTime };
    return ui.QuadPart;
}

uint32_t ProcessMetrics::NumProcessors()
{
    static uint32_t n =
    []{ SYSTEM_INFO si; GetSystemInfo(&si); return si.dwNumberOfProcessors; }();
    return n;
}

size_t ProcessMetrics::IoRate(const uint64_t cur, uint64_t& prevBytes, FILETIME& prevWall)
{
    FILETIME nowFt;  GetSystemTimeAsFileTime(&nowFt);
    const uint64_t now = FileTimeToUint(nowFt);

    const uint64_t wallDelta  = now - FileTimeToUint(prevWall);
    const uint64_t byteDelta  = cur - prevBytes;

    prevWall  = nowFt;
    prevBytes = cur;

    if (wallDelta == 0) return 0;
    return static_cast<size_t>((byteDelta * 10'000'000ULL) / wallDelta);
}

ProcessMetrics::~ProcessMetrics()
{
    std::scoped_lock lk(mtx_);
    for (const auto &s: map_ | std::views::values)
        if (s.hProcess) CloseHandle(s.hProcess);
}

// ---------------------------------------------------------------------------
// public API
// ---------------------------------------------------------------------------
bool ProcessMetrics::AddProcess(DWORD pid)
{
    std::scoped_lock lk(mtx_);

    if (map_.contains(pid))
        return true;                    // already tracking

    HANDLE local = nullptr;
    local = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
                            FALSE, pid);
    if (GetLastError() == ERROR_ACCESS_DENIED) {
        local = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    }
    if (!local) {
        std::cerr << "Failed to open process" << std::endl;
        return false;
    };

    map_.emplace(pid, Snapshot{ local });
    return true;
}

void ProcessMetrics::RemoveProcess(const DWORD pid)
{
    std::scoped_lock lk(mtx_);
    if (const auto it = map_.find(pid); it != map_.end())
    {
        if (it->second.hProcess) CloseHandle(it->second.hProcess);
        map_.erase(it);
    }
}

void ProcessMetrics::RefreshAllData()
{
    std::scoped_lock lk(mtx_);
    FILETIME nowFt; GetSystemTimeAsFileTime(&nowFt);

    for (auto it = map_.begin(); it != map_.end(); )
    {
        const Snapshot& s = it->second;
        FILETIME c,e,k,u;
        IO_COUNTERS io;

        if (!GetProcessTimes(s.hProcess,&c,&e,&k,&u) ||
            !GetProcessIoCounters(s.hProcess,&io))
        {
            // process exited – drop entry
            CloseHandle(s.hProcess);
            it = map_.erase(it);
            continue;
        }

        ++it;
    }
}

std::optional<double> ProcessMetrics::GetCpuUsage(const DWORD pid)
{
    std::scoped_lock lk(mtx_);
    Snapshot& s = Ensure(pid);

    FILETIME c,e,k,u, nowFt;
    GetSystemTimeAsFileTime(&nowFt);

    if (!GetProcessTimes(s.hProcess,&c,&e,&k,&u))
        return std::nullopt;

    const uint64_t cpu  = FileTimeToUint(k)+FileTimeToUint(u);
    const uint64_t wall = FileTimeToUint(nowFt);

    if (s.wallTime == 0) {
        s.wallTime = wall;
        s.cpuTime  = cpu;
        return std::nullopt;
    }

    const uint64_t cpuDelta  = cpu  - s.cpuTime;
    const uint64_t wallDelta = wall - s.wallTime;

    s.cpuTime  = cpu;
    s.wallTime = wall;

    if (wallDelta == 0) return std::nullopt;
    return (cpuDelta / static_cast<double>(wallDelta)) * 100.0 / NumProcessors();
}

std::optional<size_t> ProcessMetrics::GetMemoryUsage(const DWORD pid)
{
    const HANDLE h = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (!h) return std::nullopt;

    PROCESS_MEMORY_COUNTERS_EX pmc;
    const bool ok = GetProcessMemoryInfo(h, reinterpret_cast<PPROCESS_MEMORY_COUNTERS>(&pmc), sizeof(pmc));
    CloseHandle(h);
    if (!ok) return std::nullopt;
    return static_cast<size_t>(pmc.WorkingSetSize);
}

std::optional<size_t> ProcessMetrics::GetDiskReadBytesPerSec(DWORD pid)
{
    std::scoped_lock lk(mtx_);
    Snapshot& s = Ensure(pid);

    IO_COUNTERS io;
    if (!GetProcessIoCounters(s.hProcess,&io))
        return std::nullopt;

    return IoRate(io.ReadTransferCount, s.readBytes, s.lastIoWall);
}

std::optional<size_t> ProcessMetrics::GetDiskWriteBytesPerSec(DWORD pid)
{
    std::scoped_lock lk(mtx_);
    Snapshot& s = Ensure(pid);

    IO_COUNTERS io;
    if (!GetProcessIoCounters(s.hProcess,&io))
        return std::nullopt;

    return IoRate(io.WriteTransferCount, s.writeBytes, s.lastIoWall);
}

std::optional<size_t> ProcessMetrics::GetDiskIOBytesPerSec(DWORD pid)
{
    const auto r = GetDiskReadBytesPerSec(pid);
    const auto w = GetDiskWriteBytesPerSec(pid);
    if (r && w) return *r + *w;
    if (r)      return r;
    if (w)      return w;
    return std::nullopt;
}

ProcessMetrics::Snapshot& ProcessMetrics::Ensure(DWORD pid)
{
    auto it = map_.find(pid);
    if (it == map_.end())
        it = map_.emplace(pid, Snapshot{}).first;
    return it->second;
}