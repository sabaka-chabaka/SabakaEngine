#pragma once
#define NOMINMAX
#include <algorithm>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>

namespace engine {

    class ThreadPool {
    public:
        explicit ThreadPool(size_t threadCount = 0) {
            size_t n = threadCount > 0
                ? threadCount
                : (std::max)(1u, std::thread::hardware_concurrency() - 1);

            m_workers.reserve(n);
            for (size_t i = 0; i < n; ++i) {
                m_workers.emplace_back([this] { workerLoop(); });
            }
        }

        ~ThreadPool() {
            {
                std::unique_lock lock(m_mutex);
                m_stop = true;
            }
            m_cv.notify_all();
            for (auto& t : m_workers) t.join();
        }

        ThreadPool(const ThreadPool&)            = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        template<typename F, typename... Args>
        auto submit(F&& fn, Args&&... args)
            -> std::future<std::invoke_result_t<F, Args...>>
        {
            using R    = std::invoke_result_t<F, Args...>;
            auto task  = std::make_shared<std::packaged_task<R()>>(
                [fn = std::forward<F>(fn),
                 args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
                    return std::apply(std::move(fn), std::move(args));
                });

            std::future<R> fut = task->get_future();
            {
                std::unique_lock lock(m_mutex);
                m_queue.push([task] { (*task)(); });
            }
            m_cv.notify_one();
            return fut;
        }

        size_t threadCount() const { return m_workers.size(); }

    private:
        void workerLoop() {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock lock(m_mutex);
                    m_cv.wait(lock, [this] { return m_stop || !m_queue.empty(); });
                    if (m_stop && m_queue.empty()) return;
                    task = std::move(m_queue.front());
                    m_queue.pop();
                }
                task();
            }
        }

        std::vector<std::thread>          m_workers;
        std::queue<std::function<void()>> m_queue;
        std::mutex                        m_mutex;
        std::condition_variable           m_cv;
        bool                              m_stop = false;
    };

}