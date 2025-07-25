#include "JobSystem.hpp"

#include "JobGraph.hpp"

JobSystem::JobSystem(size_t threadCount)
    : _threadCount(threadCount), _frameArena(1024 * 1024), _longLivedArena(1024 * 1024) {}

void JobSystem::submit(Job& job) {}

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
