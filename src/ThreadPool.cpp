#include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t threadCount) : _threadCount(threadCount) {
  for (int i = 0; i < _threadCount; ++i) {
    std::thread t([this]() {
      while (!_stop) {
        std::unique_lock<std::mutex> workLock(_workMutex);
        _workCv.wait(workLock, [this] { return !_queue.empty() || _stop; });

        if (_stop && _queue.empty()) {
          return;
        }

        if (!_queue.empty()) {
          auto job = std::move(_queue.front());
          _queue.pop();
          workLock.unlock();
          ++_activeJobs;
          job();

          if (--_activeJobs == 0) {
            std::unique_lock<std::mutex> waitLock(_waitMutex);
            _waitCv.notify_all();
          }
        }
      }
    });
    _workers.push_back(std::move(t));
  }
}

ThreadPool::~ThreadPool() {
  _stop = true;
  _workCv.notify_all();
  _waitCv.notify_all();

  for (size_t i = 0; i < _threadCount; ++i) {
    if (_workers[i].joinable()) {
      _workers[i].join();
    }
  }
}

void ThreadPool::submit(std::function<void()> job) {
  {
    std::unique_lock<std::mutex> lock(_workMutex);
    _queue.push(std::move(job));
  }
  _workCv.notify_one();
}

void ThreadPool::waitAll() {
  std::unique_lock<std::mutex> lock(_waitMutex);
  _waitCv.wait(lock, [this] { return (_queue.empty() && _activeJobs == 0) || _stop; });
}