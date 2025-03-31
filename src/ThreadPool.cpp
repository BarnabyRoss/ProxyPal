
#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t num_size) : stop_(false){

    for(int i = 0; i < num_size; ++i)
      workers_.emplace_back(&ThreadPool::workerFunction, this);
}


void ThreadPool::workerFunction(){

    while(true){
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(queue_mutex_);
          cv_.wait(lock, [this](){ return stop_ || !tasks_queue_.empty(); });

          if( stop_ && tasks_queue_.empty() ) return;

          task = std::move(tasks_queue_.front());
          tasks_queue_.pop();
        }
        task();
    }
}


  ThreadPool::~ThreadPool(){
    {
      std::unique_lock lock(queue_mutex_);
      stop_ = true;
    }

    for(int i = 0; i < workers_.size(); ++i){
      if( workers_[i].joinable() ) workers_[i].join();
    }
  }




