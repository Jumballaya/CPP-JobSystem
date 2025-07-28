#pragma once

#include <atomic>
#include <cstdint>
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
  std::atomic<uint32_t> inDegree = 0;  // Number of inputs that must run before this node runs
  uint32_t generation = 1;             // Lines up with the handle generation
  std::atomic<bool> scheduled = false;
};

class JobGraph {
 public:
  using OnGraphCompleteFn = void (*)(GraphNodeHandle, void*);

  explicit JobGraph(FrameArena* arena, JobSystem* system);

  template <typename Node, typename... Args>
  GraphNodeHandle addNode(Args&&... args);

  void setDependencies(GraphNodeHandle node, std::initializer_list<GraphNodeHandle> deps);
  void submitReadyJobs(JobSystem& system);
  void reset();

  void setOnGraphComplete(OnGraphCompleteFn fn, void* userData);
  void onJobComplete(GraphNodeHandle node, JobSystem& system);

 private:
  FrameArena* _arena = nullptr;
  JobSystem* _system = nullptr;
  ArenaVector<JobGraphNodeSlot> _slots;

  OnGraphCompleteFn _onComplete = nullptr;
  void* _onCompleteUserData = nullptr;
};

template <typename Node, typename... Args>
GraphNodeHandle JobGraph::addNode(Args&&... args) {
  static_assert(sizeof...(Args) == 1 || sizeof...(Args) == 2);

  assert(_slots.size() < _slots.capacity() && "JobGraph capacity exceeded");
  _slots.emplace_back(_arena);
  auto& slot = _slots.back();

  auto* control = _arena->allocate<JobControlBlock>();
  control->state.store(JobState::Pending);
  control->cancelRequested.store(false);
  slot.job.control = control;

  GraphNodeHandle handle{
      .index = static_cast<uint32_t>(_slots.size() - 1),
      .generation = 1,
      .control = control};

  if constexpr (IsJobGraphNodeWithOutput<Node, std::decay_t<Args>...>) {
    const auto* input = std::get<0>(std::forward_as_tuple(args...));
    auto* output = std::get<1>(std::forward_as_tuple(args...));

    struct JobData {
      const std::decay_t<decltype(*input)>* in;
      std::decay_t<decltype(*output)>* out;
      FrameArena* arena;
      JobSystem* system;
      JobGraph* graph;
      GraphNodeHandle handle;
    };

    auto* data = _arena->allocate<JobData>();
    data->in = input;
    data->out = output;
    data->arena = _arena;
    data->system = _system;
    data->graph = this;
    data->handle = handle;

    slot.job.fn = [](void* userData) {
      auto* d = static_cast<JobData*>(userData);
      Node::run(d->in, d->out, d->arena);
    };
    slot.job.onComplete = [](void* userData) {
      auto* d = static_cast<JobData*>(userData);
      d->graph->onJobComplete(d->handle, *d->system);
    };
    slot.job.userData = data;
    slot.job.arena = _arena;
    slot.job.flags = JobFlags::None;

  } else if constexpr (IsJobGraphNodeNoOutput<Node, std::decay_t<Args>...>) {
    const auto* input = std::get<0>(std::forward_as_tuple(args...));

    struct JobData {
      const std::decay_t<decltype(*input)>* in;
      FrameArena* arena;
      JobSystem* system;
      JobGraph* graph;
      GraphNodeHandle handle;
    };

    auto* data = _arena->allocate<JobData>();
    data->in = input;
    data->arena = _arena;
    data->system = _system;
    data->graph = this;
    data->handle = handle;

    slot.job.fn = [](void* userData) {
      auto* d = static_cast<JobData*>(userData);
      Node::run(d->in, d->arena);
    };
    slot.job.onComplete = [](void* userData) {
      auto* d = static_cast<JobData*>(userData);
      d->graph->onJobComplete(d->handle, *d->system);
    };
    slot.job.userData = data;
    slot.job.arena = _arena;
    slot.job.flags = JobFlags::None;

  } else {
    static_assert(always_false<Node>, "Node does not conform to JobGraphNode concept");
  }

  return handle;
}