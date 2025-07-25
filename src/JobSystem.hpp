#pragma once

#include <cstddef>
#include <thread>
#include <vector>

#include "Job.hpp"

class JobGraph;

enum MemoryClass {
  Frame,
  LongLived
};

class JobSystem {
 public:
  JobSystem(size_t threadCount);

  void submit(Job& job);

  JobGraph createGraph(MemoryClass cls = MemoryClass::Frame);
  void submitGraph(JobGraph& graph);

  bool isComplete(const JobHandle& handle);
  bool isCancelled(const JobHandle& handle);
  bool cancel(JobHandle handle);
  void wait(JobHandle handle);

  FrameArena& frameArena();
  FrameArena& longLivedArena();

 private:
  size_t _threadCount;

  std::vector<std::thread> _threads;

  FrameArena _frameArena;
  FrameArena _longLivedArena;
};