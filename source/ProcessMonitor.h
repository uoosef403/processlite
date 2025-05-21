#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <shared_mutex>
#include <windows.h>
#include <unordered_map>
#include <thread>
#include <stop_token>

#include "ProcessMetrics.h"
#include "Concurrency/TaskDefinition.h"

class TaskManager;
struct ProcessInfo;

class ProcessMonitor {
public:
    ProcessMonitor(TaskManager& taskManager, HWND hMainWindow, HWND hListView);
    ~ProcessMonitor();

    // Disable copy/move
    ProcessMonitor(const ProcessMonitor&) = delete;
    ProcessMonitor& operator=(const ProcessMonitor&) = delete;
    ProcessMonitor(ProcessMonitor&&) = delete;
    ProcessMonitor& operator=(ProcessMonitor&&) = delete;

    void stopMonitoring();
    const ProcessInfo* getProcessInfo(DWORD pid) const;

private:
    mutable std::shared_mutex processes_mutex_;
    std::unordered_map<DWORD, ProcessInfo> processes_;
    std::stop_source stop_source_;
    std::jthread monitor_thread_;

    TaskManager& task_manager_;
    HWND hwnd_main_window_;
    HWND hwnd_list_view_;
    TaskId monitoring_task_id_ = -1;
    ProcessMetrics p_metrics_;

    void scheduledUpdateProcesses(const std::stop_token &st);
};

#endif // PROCESS_MONITOR_H