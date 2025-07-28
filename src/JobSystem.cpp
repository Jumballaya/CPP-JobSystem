#include "JobSystem.hpp"

#include "JobGraph.hpp"

void WorkerThread::run() {
  ThreadArenaRegistry::set(&localArena);

  while (running) {
    Job job;

    if (queue.try_dequeue(job)) {
      if (job.fn) {
        job.fn(job.userData);
      }
      if (job.control) {
        bool wasCancelled = job.control->cancelRequested.load(std::memory_order_relaxed);
        JobState newState = wasCancelled ? JobState::Cancelled : JobState::Completed;
        job.control->state.store(newState, std::memory_order_release);
      }
      if (job.onComplete) {
        job.onComplete(job.userData);
      }
      continue;
    }

    if (system && system->getNextJob(job)) {
      if (job.fn) {
        job.fn(job.userData);
      }
      if (job.onComplete) {
        job.onComplete(job.userData);
      }
      continue;
    }

    std::this_thread::yield();
  }
}

JobSystem::JobSystem(size_t threadCount)
    : _frameArena(1024 * 1024), _longLivedArena(1024 * 1024), _internalArena(1024 * 1024), _threadCount(threadCount), _workers(_threadCount), _globalQueue(512, &_internalArena), _highPriorityQueue(512, &_internalArena) {
  for (size_t i = 0; i < _threadCount; ++i) {
    auto worker = std::make_unique<WorkerThread>(512 * 1024);
    worker->index = i;
    worker->system = this;
    worker->thread = std::thread([workerPtr = worker.get()]() {
      workerPtr->run();
    });
    _workers.emplace_back(std::move(worker));
  }
}

JobSystem::~JobSystem() {
  for (auto& worker : _workers) {
    worker->running = false;
  }

  for (auto& worker : _workers) {
    if (worker->thread.joinable()) {
      worker->thread.join();
    }
  }

  _highPriorityQueue.shutdown();
  _globalQueue.shutdown();
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
  graph.submitReadyJobs();
}

bool JobSystem::isComplete(const JobHandle& handle) {
  if (!handle.isValid()) return false;
  JobState state = handle.control->state.load(std::memory_order_acquire);
  return state == JobState::Completed || state == JobState::Cancelled;
}

bool JobSystem::isCancelled(const JobHandle& handle) {
  if (!handle.isValid()) return false;
  return handle.control->cancelRequested.load(std::memory_order_acquire);
}

bool JobSystem::cancel(JobHandle handle) {
  // @TODO: Check for JobFlags::Cancelable (add the JobFlags to the control block?)
  if (!handle.isValid()) return false;
  handle.control->cancelRequested.store(true, std::memory_order_release);
  return true;
}

void JobSystem::wait(JobHandle handle) {
  if (!handle.isValid()) return;
  while (true) {
    JobState state = handle.control->state.load(std::memory_order_acquire);
    if (state == JobState::Completed || state == JobState::Cancelled) return;
    std::this_thread::yield();
  }
}

FrameArena& JobSystem::frameArena() { return _frameArena; }
FrameArena& JobSystem::longLivedArena() { return _longLivedArena; }
