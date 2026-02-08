#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class ThreadPool {
public:
    explicit ThreadPool(size_t threadCount)
        : stop(false)
    {
        for (size_t i = 0; i < threadCount; ++i) {
            workers.emplace_back([this]() {
                workerLoop();
                });
        }
    }

    ~ThreadPool() {
        shutdown();
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(ThreadPool&&) = default;
    ThreadPool& operator=(ThreadPool&&) = default;

    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mutex);

            if (stop)
                return;

            tasks.push_front(std::move(task));
        }

        cv.notify_one();
    }

private:
    void workerLoop() {
        while (true) {
            std::function<void()> task;

            {
                std::unique_lock<std::mutex> lock(mutex);

                cv.wait(lock, [this]() {
                    return stop || !tasks.empty();
                    });

                if (stop && tasks.empty())
                    return;

                task = std::move(tasks.front());
                tasks.pop_front();
            }

            task();
        }
    }

    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mutex);
            stop = true;
        }

        cv.notify_all();

        for (auto& t : workers) {
            if (t.joinable())
                t.join();
        }
    }

private:
    std::vector<std::thread> workers;
    std::deque<std::function<void()>> tasks;

    std::mutex mutex;
    std::condition_variable cv;

    bool stop;
};

#endif
