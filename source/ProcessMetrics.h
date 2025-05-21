#ifndef PM_H
#define PM_H

#include <windows.h>
#include <optional>
#include <unordered_map>
#include <mutex>

class ProcessMetrics
{
public:
    ProcessMetrics()  = default;
    ~ProcessMetrics();

    /// Register a process we should track.
    /// If you already opened the process, pass the handle; otherwise pass nullptr.
    /// Returns false if the PID/handle is invalid.
    bool AddProcess(DWORD pid);

    /// Hint that the process is gone.
    void RemoveProcess(DWORD pid);

    /// Refresh cached kernel/user + I/O byte counts for **all** registered PIDs.
    void RefreshAllData();

    // ---- per‑process queries -----------------------------------------------
    std::optional<double> GetCpuUsage(DWORD pid);          // %
    static std::optional<size_t> GetMemoryUsage(DWORD pid);// bytes
    std::optional<size_t> GetDiskReadBytesPerSec(DWORD pid);
    std::optional<size_t> GetDiskWriteBytesPerSec(DWORD pid);
    std::optional<size_t> GetDiskIOBytesPerSec(DWORD pid);

private:
    struct Snapshot
    {
        HANDLE   hProcess  = nullptr;   // owned (must CloseHandle)
        uint64_t wallTime  = 0;         // last sample, 100‑ns
        uint64_t cpuTime   = 0;         // kernel+user, 100‑ns
        uint64_t readBytes = 0;         // cumulative
        uint64_t writeBytes= 0;         // cumulative
        FILETIME lastIoWall{};          // helper for B/s maths
    };

    // helpers ----------------------------------------------------------------
    static uint64_t FileTimeToUint(const FILETIME& ft);
    static uint32_t NumProcessors();

    static size_t          IoRate(uint64_t cur, uint64_t& prevBytes, FILETIME& prevWall);
    Snapshot&       Ensure(DWORD pid);

    // data -------------------------------------------------------------------
    std::mutex                           mtx_;
    std::unordered_map<DWORD, Snapshot>  map_;
};

#endif // PM_H
