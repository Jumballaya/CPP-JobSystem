#pragma once

#include <initializer_list>
#include <span>
#include <utility>

#include "ArenaVector.hpp"
#include "Job.hpp"
#include "JobGraphNode.hpp"

class JobSystem;

struct JobGraphNodeSlot {
  JobGraphNodeSlot(FrameArena* arena) : dependents(arena, 2), inDegree(0), generation(0), scheduled(false) {}

  Job job;
  ArenaVector<GraphNodeHandle> dependents;  // List of nodes that depend on this node
  uint32_t inDegree = 0;                    // Number of inputs that must run before this node runs
  uint32_t generation = 1;                  // Lines up with the handle generation
  bool scheduled = false;                   // Has this node been submitted and scheduled to run?
};

class JobGraph {
 public:
  explicit JobGraph(FrameArena* arena);

  template <typename Node, typename... Args>
  GraphNodeHandle addNode(Args&&... args);

  void setDependencies(GraphNodeHandle node, std::initializer_list<GraphNodeHandle> deps);
  void submitReadyJobs(JobSystem& system);
  void reset();

 private:
  FrameArena* _arena = nullptr;
  ArenaVector<JobGraphNodeSlot> _slots;
};

template <typename Node, typename... Args>
GraphNodeHandle JobGraph::addNode(Args&&... args) {
  assert(_slots.size() < _capacity && "JobGraph capacity exceeded");
  _slots.emplace_back(_arena);
  auto& slot = _slots[_slots.size() - 1];
  slot.job = makeJob<Node>(std::forward<Args>(args)...);

  GraphNodeHandle handle{.index = static_cast<uint32_t>(_slots.size() - 1), .generation = 1};
  return handle;
}