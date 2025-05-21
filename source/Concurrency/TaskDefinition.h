#ifndef TaskDefinition_h
#define TaskDefinition_h

#include <chrono>
#include <functional>
#include <stop_token>
#include <string>

using TaskId = std::size_t;

struct TaskDefinition {
    using TaskFunction = std::function<void(std::stop_token)>;

    TaskId id;
    std::string name;
    TaskFunction function;
    std::chrono::milliseconds interval;
    std::chrono::steady_clock::time_point next_execution;
    std::stop_source stop_source;

    // Constructor
    TaskDefinition(
        const TaskId id,
        const std::string &name,
        TaskFunction function,
        const std::chrono::milliseconds in_interval
    ): id(id),
       name(name),
       function(std::move(function)),
       interval(in_interval),
       next_execution(std::chrono::steady_clock::now() + in_interval) {
    }

    // Rule 6(and 5) of c++ 11+
    TaskDefinition(const TaskDefinition &) = delete;

    TaskDefinition &operator=(const TaskDefinition &) = delete;

    TaskDefinition(TaskDefinition &&) = default;

    TaskDefinition &operator=(TaskDefinition &&) = default;
};

#endif
