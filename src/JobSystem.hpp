#pragma once

#include <atomic>
#include <cstddef>
#include <thread>
#include <utility>
#include <vector>

#include "ArenaVector.hpp"
#include "Job.hpp"
#include "LockFreeQueue.hpp"
#include "ThreadArenaRegistry.hpp"

class JobGraph;

enum MemoryClass {
  Frame,
  LongLived
};

struct WorkerThread {
  WorkerThread() : queue(256), localArena(512 * 1024) {}

  ~WorkerThread() = default;
  WorkerThread(const WorkerThread&) = delete;
  WorkerThread& operator=(const WorkerThread&) = delete;
  WorkerThread(WorkerThread&&) = default;
  WorkerThread& operator=(WorkerThread&&) = default;

  std::thread thread;
  LockFreeQueue<Job> queue;
  FrameArena localArena;

  size_t index = 0;
  std::atomic<bool> running = true;

  JobSystem* system = nullptr;

  void run() {
    ThreadArenaRegistry::set(&localArena);

    while (running) {
      Job job;

      if (queue.try_dequeue(job)) {
        job.fn(job.userData);
        if (job.onComplete) {
          job.onComplete(job.userData);
        }
        continue;
      }

      if (system && system->getNextJob(job)) {
        job.fn(job.userData);
        if (job.onComplete) {
          job.onComplete(job.userData);
        }
        continue;
      }

      std::this_thread::yield();
    }
  }
};

class JobSystem {
 public:
  JobSystem(size_t threadCount);

  ~JobSystem();

  void submit(Job& job);

  JobGraph createGraph(MemoryClass cls = MemoryClass::Frame);
  void submitGraph(JobGraph& graph);

  bool isComplete(const JobHandle& handle);
  bool isCancelled(const JobHandle& handle);
  bool cancel(JobHandle handle);
  void wait(JobHandle handle);

  FrameArena& frameArena();
  FrameArena& longLivedArena();

  bool getNextJob(Job& out);

 private:
  FrameArena _frameArena;
  FrameArena _longLivedArena;
  FrameArena _internalArena;

  size_t _threadCount;

  ArenaVector<WorkerThread> _workers;

  LockFreeQueue<Job> _globalQueue;
  LockFreeQueue<Job> _highPriorityQueue;
};