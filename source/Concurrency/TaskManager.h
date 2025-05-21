#ifndef TaskManager_h
#define TaskManager_h
#include <shared_mutex>

#include "TaskDefinition.h"

class TaskManager {
public:
    TaskManager() = default;
    ~TaskManager() = default;

    // Rule 5/6: Disable copy and move
    TaskManager(const TaskManager &) = delete;
    TaskManager &operator=(const TaskManager &) = delete;
    TaskManager(TaskManager &&) = delete;
    TaskManager &operator=(TaskManager &&) = delete;

    TaskId addTask(TaskId taskId, std::string name, TaskDefinition::TaskFunction function,
                   std::chrono::milliseconds interval);

    bool removeTask(TaskId id);

    bool stopTask(TaskId id);

    void stopAllTasks();

    void updateTaskNextRunTime(TaskId id, std::chrono::steady_clock::time_point newTime);

    std::vector<std::shared_ptr<TaskDefinition>> getAllTasksAsSnapshot() const;

private:
    mutable std::shared_mutex tasks_mutex_;
    std::unordered_map<TaskId, std::shared_ptr<TaskDefinition> > tasks_;
};

#endif
