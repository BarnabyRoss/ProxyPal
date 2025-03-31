
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <iostream>
#include <queue>
#include <vector>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <exception>

class ThreadPool{

private:
    std::queue< std::function<void()> > tasks_queue_;
    std::vector<std::thread> workers_;

    //同步源语
    std::mutex queue_mutex_;
    std::condition_variable cv_;

    bool stop_;

public:
    ThreadPool(size_t num_size);

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator= (const ThreadPool&) = delete;

    template <typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future< std::invoke_result_t<F, Args...> >{

        using return_type = std::invoke_result<F, Args...>;

        auto task = std::make_shared< packaged_task<return_type()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();

        {
          std::unique_lock lock(queue_mutex_);

          if( stop_ ) throw std;:runtime_error("thread pool stoped!");

          tasks_queue_.emplace([task](){ (*task)(); });
        }

        cv_.notify_one();

        return res;
    }

    ~ThreadPool();

private:
  void workerFunction();

};


#endif
