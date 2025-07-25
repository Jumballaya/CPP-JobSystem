#pragma once

#include <initializer_list>
#include <span>
#include <utility>

#include "ArenaVector.hpp"
#include "Job.hpp"
#include "JobGraphNode.hpp"
#include "JobSystem.hpp"

struct JobGraphNodeSlot {
  JobGraphNodeSlot(FrameArena* arena) : dependents(arena, 2), inDegree(0), generation(0), scheduled(false) {}

  Job job;
  ArenaVector<GraphNodeHandle> dependents;
  uint32_t inDegree = 0;    // Number of inputs that must run before this node runs
  uint32_t generation = 1;  // Lines up with the handle generation
  bool scheduled = false;
};

class JobGraph {
 public:
  using OnGraphCompleteFn = void (*)(GraphNodeHandle, void*);

  explicit JobGraph(FrameArena* arena);

  template <typename Node, typename... Args>
  GraphNodeHandle addNode(JobSystem& system, Args&&... args);

  void setDependencies(GraphNodeHandle node, std::initializer_list<GraphNodeHandle> deps);
  void submitReadyJobs(JobSystem& system);
  void reset();

  void setOnGraphComplete(OnGraphCompleteFn fn, void* userData);
  void onJobComplete(GraphNodeHandle node, JobSystem& system);

 private:
  FrameArena* _arena = nullptr;
  ArenaVector<JobGraphNodeSlot> _slots;

  OnGraphCompleteFn _onComplete = nullptr;
  void* _onCompleteUserData = nullptr;
};

template <typename Node, typename... Args>
GraphNodeHandle JobGraph::addNode(JobSystem& system, Args&&... args) {
  assert(_slots.size() < _slots.capacity() && "JobGraph capacity exceeded");
  _slots.emplace_back(_arena);
  auto& slot = _slots[_slots.size() - 1];
  slot.job = makeJob<Node>(std::forward<Args>(args)...);

  GraphNodeHandle handle{.index = static_cast<uint32_t>(_slots.size() - 1), .generation = 1};
  return handle;
}