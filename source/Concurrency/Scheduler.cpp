#include "Scheduler.h"

#include <iostream>

#include "TaskManager.h"
#include "ThreadPool.h"

Scheduler::Scheduler(TaskManager &tm, ThreadPool &tp, const std::chrono::milliseconds pr)
    : task_mgr_(tm), thread_pool_(tp), precision_(pr),
      scheduler_thread_([this](const std::stop_token &st) {
          scheduler_loop(st);
      }, scheduler_stop_source_.get_token()) {
    if (precision_ <= 0ms) {
        precision_ = 50ms;
    }
}

Scheduler::~Scheduler() {
    if (!scheduler_stopped_.exchange(true)) {
        scheduler_stop_source_.request_stop();
    }
}

void Scheduler::scheduler_loop(const std::stop_token &st) const {
    while (!st.stop_requested()) {
        auto now = std::chrono::steady_clock::now();
        const auto defaultWakeupTimeout = now + precision_;
        auto earliestNextRun = defaultWakeupTimeout;

        auto taskSnapshot = task_mgr_.getAllTasksAsSnapshot();

        for (const auto &task: taskSnapshot) {
            if (task->stop_source.stop_requested()) {
                continue;
            }

            if (now >= task->next_execution) {
                std::cout << "Executing task " << task->name <<std::endl;
                const auto nextExecutionForThisTask = now + task->interval;
                task_mgr_.updateTaskNextRunTime(task->id, nextExecutionForThisTask);

                auto funcToRun = task->function;
                auto taskStopToken = task->stop_source.get_token();
                auto taskName = task->name;
                thread_pool_.enqueue([
                    funcToRun,
                    taskStopToken,
                    taskName] {
                    if (taskStopToken.stop_requested()) {
                        return;
                    }
                    try {
                        funcToRun(taskStopToken);
                    } catch (const std::exception& e) {
                        std::cerr << "Scheduler->Pool: Task '" << taskName << "'"
                                  << " threw exception: " << e.what() << '\n';
                    } catch (...) {
                        std::cerr << "Scheduler->Pool: Task '" << taskName << "'"
                                  << " threw unknown exception.\n";
                    }
                });

                earliestNextRun = std::min(earliestNextRun, nextExecutionForThisTask);
            } else {
                earliestNextRun = std::min(earliestNextRun, task->next_execution);
            }

            if (earliestNextRun > std::chrono::steady_clock::now()) {
                std::this_thread::sleep_until(earliestNextRun);
            } else {
                std::this_thread::yield();
            }
        }
    }
}
