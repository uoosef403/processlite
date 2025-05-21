#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <chrono>
#include <stop_token>
#include <thread>

class TaskManager;
class ThreadPool;

using namespace std::chrono_literals;

class Scheduler {
public:
    Scheduler(TaskManager& tm, ThreadPool& tp, std::chrono::milliseconds precision = 50ms);

    ~Scheduler();

    // Rule of 5/6: Disable copy/move semantics.
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;

private:
    void scheduler_loop(const std::stop_token &st) const;

    TaskManager& task_mgr_;
    ThreadPool& thread_pool_;
    std::chrono::milliseconds precision_{50ms};
    std::stop_source scheduler_stop_source_;
    std::jthread scheduler_thread_;
    std::atomic<bool> scheduler_stopped_;
};

#endif