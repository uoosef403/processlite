#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
public:
    explicit ThreadPool(std::size_t num_threads = std::thread::hardware_concurrency());

    ~ThreadPool();

    void stop();
    void enqueue(std::function<void()> task);
    void workerLoop(const std::stop_token &st);

private:
    std::vector<std::jthread> worker_threads_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::stop_source stop_all_;
    std::atomic<bool> stopped_{false};
};

#endif