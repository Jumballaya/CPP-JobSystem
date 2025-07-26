#include "JobSystem.hpp"

#include "JobGraph.hpp"

JobSystem::JobSystem(size_t threadCount)
    : _frameArena(1024 * 1024), _longLivedArena(1024 * 1024), _internalArena(1024 * 1024), _threadCount(threadCount), _workers(&_internalArena, _threadCount), _globalQueue(512, &_internalArena), _highPriorityQueue(512, &_internalArena) {
  for (size_t i = 0; i < _threadCount; ++i) {
    WorkerThread worker;
    worker.index = i;
    worker.system = this;
    worker.thread = std::thread([&worker]() { worker.run(); });
    _workers.emplace_back(std::move(worker));
  }
}

JobSystem::~JobSystem() {
  for (auto& worker : _workers) {
    worker.running = false;
  }

  for (auto& worker : _workers) {
    if (worker.thread.joinable()) {
      worker.thread.join();
    }
  }
}

bool JobSystem::getNextJob(Job& out) {
  if (_highPriorityQueue.try_dequeue(out)) {
    return true;
  }
  if (_globalQueue.try_dequeue(out)) {
    return true;
  }
  return false;
}

void JobSystem::submit(Job& job) {
  if (HasFlag(job.flags, JobFlags::HighPriority)) {
    while (!_highPriorityQueue.try_enqueue(std::move(job))) {
      std::this_thread::yield();
    }
  } else {
    while (!_globalQueue.try_enqueue(std::move(job))) {
      std::this_thread::yield();
    }
  }
}

JobGraph JobSystem::createGraph(MemoryClass cls) {
  switch (cls) {
    case MemoryClass::Frame:
      return JobGraph(&_frameArena, this);
    case MemoryClass::LongLived:
      return JobGraph(&_longLivedArena, this);
  }
  return JobGraph(&_frameArena, this);
}

void JobSystem::submitGraph(JobGraph& graph) {
  graph.submitReadyJobs(*this);
}

bool JobSystem::isComplete(const JobHandle& handle) { return false; }
bool JobSystem::isCancelled(const JobHandle& handle) { return false; }
bool JobSystem::cancel(JobHandle handle) { return false; }
void JobSystem::wait(JobHandle handle) {}

FrameArena& JobSystem::frameArena() { return _frameArena; }
FrameArena& JobSystem::longLivedArena() { return _longLivedArena; }
