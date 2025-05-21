#include "ProcessMonitor.h"
#include <tlhelp32.h>
#include <shlwapi.h>
#include <chrono>
#include <iostream>

#include "HandleWrapper.h"
#include "ProcessInfo.h"
#include "TasksIDDef.h"
#include "Concurrency/TaskManager.h"

using namespace std::chrono_literals;

ProcessMonitor::ProcessMonitor(TaskManager &taskManager, const HWND hMainWindow, const HWND hListView)
    : task_manager_(taskManager),
      hwnd_main_window_(hMainWindow),
      hwnd_list_view_(hListView) {
    if (!IsWindow(hwnd_main_window_) || !IsWindow(hwnd_list_view_)) {
        std::cerr << "Invalid window or list view handle" << std::endl;
    }

    try {
        monitoring_task_id_ = task_manager_.addTask(
            ProcessMonitorScheduledUpdateTaskID,
            "Process Monitor Update", // Task name
            [this](const std::stop_token &st) { this->scheduledUpdateProcesses(st); },
            1000ms
        );
    } catch (...) {
        std::cerr << "Something went wrong when creating new task" << std::endl;
        monitoring_task_id_ = -1;
    }
}

ProcessMonitor::~ProcessMonitor() {
    stopMonitoring();
}

void ProcessMonitor::stopMonitoring() {
    stop_source_.request_stop();
}

const ProcessInfo *ProcessMonitor::getProcessInfo(const DWORD pid) const {
    const auto it = processes_.find(pid);
    return (it != processes_.end()) ? &it->second : nullptr;
}

void ProcessMonitor::scheduledUpdateProcesses(const std::stop_token &st) {
    HandleWrapper hSnapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
    if (!hSnapshot.isValid()) {
        return;
    }

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);

    if (!Process32FirstW(hSnapshot, &pe32)) {
        return;
    }

    std::unordered_map<DWORD, ProcessInfo> currentSystemProcesses;
    const auto now = std::chrono::steady_clock::now();

    do {
        if (st.stop_requested()) {
            return;
        }
        DWORD pid = pe32.th32ProcessID;
        if (pid == 0) {
            continue;
        }

        ProcessInfo currentInfo;
        currentInfo.pid = pid;
        currentInfo.name = pe32.szExeFile;

        HandleWrapper hProcess(OpenProcess(
                PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ,
                FALSE,
                pid
            )
        );

        if (hProcess.isValid()) {
            wchar_t path[MAX_PATH];
            DWORD size = MAX_PATH;
            if (QueryFullProcessImageNameW(hProcess, 0, path, &size)) {
                currentInfo.path = path;
            }
            CloseHandle(hProcess);
        }

        currentInfo.cpuUsage = 0.0;
        currentInfo.ioRate = 0.0;
        currentInfo.ramUsage = 3;
        currentInfo.iconIndex = -1;

        currentSystemProcesses[pid] = std::move(currentInfo);
    } while (Process32NextW(hSnapshot, &pe32));

    auto updateData = std::make_unique<ProcessUpdateData>();
    std::vector<DWORD> currentPids;

    std::unique_lock lock(processes_mutex_);

    for (auto &[pid, currentInfo]: currentSystemProcesses) {
        currentPids.push_back(pid);
        p_metrics_.RefreshAllData();

        // ADDED
        if (auto it = processes_.find(pid); it == processes_.end()) {
            currentInfo.cpuUsage = 0.0;
            currentInfo.ioRate = 0.0;
            currentInfo.ramUsage = 1;
            processes_.emplace(pid, currentInfo);
            p_metrics_.AddProcess(pid);
            updateData->added.push_back(currentInfo);
        } else {
            ProcessInfo &previousInfo = it->second;

            if (
                auto timeDelta = std::chrono::duration<double>((now - previousInfo.lastUpdateTime).count());
                timeDelta.count() > 0
            ) {
                if (
                    const auto cpuUsage = p_metrics_.GetCpuUsage(pid);
                    cpuUsage.has_value()
                ) {
                    currentInfo.cpuUsage = cpuUsage.value();
                }
                if (
                    const auto ramUsage = p_metrics_.GetMemoryUsage(pid);
                    ramUsage.has_value()
                ) {
                    currentInfo.ramUsage = ramUsage.value();
                }
                if (
                    const auto diskUsage = p_metrics_.GetDiskIOBytesPerSec(pid);
                    diskUsage.has_value()
                ) {
                    currentInfo.ioRate = diskUsage.value();
                }
            } else {
                currentInfo.cpuUsage = previousInfo.cpuUsage;
                currentInfo.ramUsage = previousInfo.ramUsage;
                currentInfo.ioRate = previousInfo.ioRate;
            }

            const bool changed = previousInfo.name != currentInfo.name ||
                                 previousInfo.path != currentInfo.path ||
                                 previousInfo.ramUsage != currentInfo.ramUsage ||
                                 std::abs(previousInfo.cpuUsage - currentInfo.cpuUsage) > 0.1 ||
                                 // Threshold for CPU change
                                 std::abs(previousInfo.ioRate - currentInfo.ioRate) > 1024;
            // Threshold for I/O change (e.g., 1KB/s)

            previousInfo = currentInfo;

            if (changed) {
                updateData->updated.push_back(currentInfo);
            }
        }
    }
    // DELETED
    for (auto it = processes_.begin(); it != processes_.end();) {
        DWORD pid = it->first;
        bool found = false;
        for (DWORD currentPid: currentPids) {
            if (pid == currentPid) {
                found = true;
                break;
            }
        }

        if (!found) {
            // Process Removed(closed, killed or else)
            updateData->removed_pids.push_back(pid);
            it = processes_.erase(it); // Remove from our internal map
            p_metrics_.RemoveProcess(pid);
        } else {
            ++it; // Only increment if not erased
        }
    }
    lock.unlock();

    if (!updateData->added.empty() || !updateData->removed_pids.empty() || !updateData->updated.empty()) {
        if (
            ProcessUpdateData *rawDataPtr = updateData.release();
            !PostMessageW(hwnd_main_window_, WM_APP + 104, 0, reinterpret_cast<LPARAM>(rawDataPtr))
        ) {
            std::cerr << "ProcessMonitor: Failed to post WM_PROCESS_UPDATE message. Error: " << GetLastError() <<
                    std::endl;
            delete rawDataPtr; // Manually delete if PostMessage failed
        }
    }
}
