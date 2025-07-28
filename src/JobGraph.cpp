#include "JobGraph.hpp"

#include "JobSystem.hpp"

JobGraph::JobGraph(FrameArena* arena, JobSystem* system) : _arena(arena), _system(system), _slots(arena, 256) {}

void JobGraph::setDependencies(GraphNodeHandle node, std::initializer_list<GraphNodeHandle> deps) {
  assert(node.index < _slots.size());
  JobGraphNodeSlot& slot = _slots[node.index];
  slot.inDegree = static_cast<uint32_t>(deps.size());

  for (auto dep : deps) {
    assert(dep.index < _slots.size());
    _slots[dep.index].dependents.push_back(node);
  }
}

void JobGraph::reset() {
  _slots.clear();
}

void JobGraph::submitReadyJobs(JobSystem& system) {
  for (auto& slot : _slots) {
    if (slot.inDegree == 0 && !slot.scheduled) {
      system.submit(slot.job);
      slot.scheduled = true;
    }
  }
}

void JobGraph::onJobComplete(GraphNodeHandle node, JobSystem& system) {
  assert(node.index < _slots.size());
  JobGraphNodeSlot& slot = _slots[node.index];

  for (GraphNodeHandle dep : slot.dependents) {
    assert(dep.index < _slots.size());
    JobGraphNodeSlot& depSlot = _slots[dep.index];

    uint32_t prev = depSlot.inDegree.fetch_sub(1, std::memory_order_acq_rel);
    assert(prev > 0);

    if (prev == 1) {
      bool expected = false;
      if (depSlot.scheduled.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
        system.submit(depSlot.job);
      }
    }
  }
}

void JobGraph::setOnGraphComplete(OnGraphCompleteFn fn, void* userData) {
  _onComplete = fn;
  _onCompleteUserData = userData;
}