#include "ThreadPool.h"

#include <iostream>

ThreadPool::ThreadPool(std::size_t num_threads) {
    if (num_threads == 0) {
        num_threads = 1;
    }

    worker_threads_.reserve(num_threads);
    for (std::size_t i = 0; i < num_threads; i++) {
        worker_threads_.emplace_back([this](const std::stop_token &st) {
            workerLoop(st);
        }, stop_all_.get_token());
    }
}

ThreadPool::~ThreadPool() {
    if (!stopped_.exchange(true)) {
        stop_all_.request_stop();
        condition_.notify_all();
    }
}

void ThreadPool::stop() {
    if (!stopped_.exchange(true)) {
        stop_all_.request_stop();
        condition_.notify_all();
    }
}

void ThreadPool::enqueue(std::function<void()> task) {
    if (stopped_.load()) {
        return;
    }
    {
        std::unique_lock lock(queue_mutex_);
        tasks_.emplace(std::move(task));
    }
    condition_.notify_one();
}

void ThreadPool::workerLoop(const std::stop_token &st) {
    while (!st.stop_requested()) {
        std::function<void()> task;
        {
            std::unique_lock lock(queue_mutex_);
            condition_.wait(lock, [&] {
                return st.stop_requested() || !tasks_.empty();
            });

            if (st.stop_requested()) {
                return;
            }

            if (!tasks_.empty()) {
                task = std::move(tasks_.front());
                tasks_.pop();
            }
        }
        try {
            task();
        } catch (const std::exception& e) {
            std::cerr << "ThreadPool: Worker thread [" << std::this_thread::get_id()
                      << "] caught exception: " << e.what() << '\n';
        } catch (...) {
            std::cerr << "ThreadPool: Worker thread [" << std::this_thread::get_id()
                      << "] caught unknown exception.\n";
        }
    }
}