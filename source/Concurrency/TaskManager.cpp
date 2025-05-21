#include "TaskManager.h"

#include <iostream>
#include <ranges>

using namespace std::chrono_literals;

TaskId TaskManager::addTask(TaskId taskId, std::string name, TaskDefinition::TaskFunction function,
                            std::chrono::milliseconds interval) {
    if (interval <= 0ms) {
        std::cerr << "Interval must be greater than zero" << std::endl;
    }

    const auto taskPtr = std::make_shared<TaskDefinition>(taskId, std::move(name), std::move(function), interval);

    taskPtr->next_execution = std::chrono::steady_clock::now();

    {
        std::unique_lock lock(tasks_mutex_);
        tasks_.emplace(taskId, std::move(taskPtr));
    }

    return taskId;
}

bool TaskManager::stopTask(const TaskId id) {
    std::shared_ptr<TaskDefinition> taskToStop;
    {
        std::shared_lock lock(tasks_mutex_);
        const auto it = tasks_.find(id);
        if (it == tasks_.end()) {
            return false;
        }
        taskToStop = it->second;
    }
    if (taskToStop) {
        taskToStop->stop_source.request_stop();
        return true;
    }
    return false;
}

bool TaskManager::removeTask(const TaskId id) {
    std::shared_ptr<TaskDefinition> taskToRemove;
    bool removed = false;
    {
        std::shared_lock lock(tasks_mutex_);
        if (const auto it = tasks_.find(id); it != tasks_.end()) {
            taskToRemove = it->second;
            tasks_.erase(it);
            removed = true;
        }
    }
    if (taskToRemove) {
        taskToRemove->stop_source.request_stop();
    }
    return removed;
}

void TaskManager::stopAllTasks() {
    std::vector<std::shared_ptr<TaskDefinition>> tasksToStop;
    {
        std::shared_lock lock(tasks_mutex_);
        tasksToStop.reserve(tasks_.size());
        for (const auto &val: tasks_ | std::views::values) {
            tasksToStop.push_back(val);
        }
    }

    for (auto const &val: tasksToStop) {
        if (val) {
            val->stop_source.request_stop();
        }
    }
}

void TaskManager::updateTaskNextRunTime(const TaskId id, const std::chrono::steady_clock::time_point newTime) {
    std::unique_lock lock(tasks_mutex_);
    if (const auto it = tasks_.find(id); it != tasks_.end()) {
        it->second->next_execution = newTime;
    }
}

std::vector<std::shared_ptr<TaskDefinition>> TaskManager::getAllTasksAsSnapshot() const {
    std::vector<std::shared_ptr<TaskDefinition>> snapshot;
    std::shared_lock lock(tasks_mutex_);
    snapshot.reserve(tasks_.size());
    for (const auto &val: tasks_ | std::views::values) {
        snapshot.push_back(val);
    }
    return snapshot;
}
