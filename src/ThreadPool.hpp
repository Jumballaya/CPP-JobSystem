#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

class ThreadPool {
 public:
  explicit ThreadPool(size_t threadCount);
  ~ThreadPool();

  void submit(std::function<void()> job);
  void waitAll();

 private:
  size_t _threadCount;
  std::queue<std::function<void()>> _queue;
  std::vector<std::thread> _workers;
  std::mutex _workMutex;
  std::mutex _waitMutex;
  std::condition_variable _workCv;
  std::condition_variable _waitCv;
  std::atomic<bool> _stop{false};
  std::atomic<size_t> _activeJobs{0};
};