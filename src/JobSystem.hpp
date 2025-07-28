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

class JobSystem;
struct WorkerThread {
  WorkerThread() : localArena(512 * 1024), queue(256, &localArena) {}

  explicit WorkerThread(size_t arenaSize)
      : localArena(arenaSize), queue(256, &localArena) {}

  ~WorkerThread() = default;
  WorkerThread(const WorkerThread&) = delete;
  WorkerThread& operator=(const WorkerThread&) = delete;
  WorkerThread(WorkerThread&&) = default;
  WorkerThread& operator=(WorkerThread&&) = default;

  std::thread thread;
  FrameArena localArena;
  LockFreeQueue<Job> queue;

  size_t index = 0;
  std::atomic<bool> running = true;

  JobSystem* system = nullptr;

  void run();
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

  // @TODO: Use a PMR vector and custom allocator for the WorkerThreads to avoid all of this
  std::vector<std::unique_ptr<WorkerThread>> _workers;

  LockFreeQueue<Job> _globalQueue;
  LockFreeQueue<Job> _highPriorityQueue;
};