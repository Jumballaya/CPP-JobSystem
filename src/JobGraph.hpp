#pragma once

#include <initializer_list>
#include <span>
#include <utility>

#include "Job.hpp"
#include "JobGraphNode.hpp"

class JobSystem;

struct JobGraphNodeSlot {
  Job job;
  std::span<GraphNodeHandle> dependents;  // List of nodes that depend on this node
  uint32_t inDegree = 0;                  // Number of inputs that must run before this node runs
  uint32_t generation = 1;                // Lines up with the handle generation
  bool scheduled = false;                 // Has this node been submitted and scheduled to run?
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

  JobGraphNodeSlot* _slots = nullptr;
  size_t _capacity = 0;
  size_t _count = 0;

  std::span<JobGraphNodeSlot> slots() { return {_slots, _count}; }
  std::span<const JobGraphNodeSlot> slots() const { return {_slots, _count}; }
};

template <typename Node, typename... Args>
GraphNodeHandle JobGraph::addNode(Args&&... args) {
  assert(_count < _capacity && "JobGraph capacity exceeded");
  auto& slot = _slots[_count];
  slot.job = makeJob<Node>(std::forward<Args>(args)...);
  slot.generation = 1;
  slot.scheduled = false;
  slot.inDegree = 0;
  slot.dependents = std::span<GraphNodeHandle>{};
  GraphNodeHandle handle{.index = static_cast<uint32_t>(_count), .generation = 1};
  ++_count;

  return handle;
}