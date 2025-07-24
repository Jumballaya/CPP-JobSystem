#include "JobGraph.hpp"

#include "JobSystem.hpp"

JobGraph::JobGraph(FrameArena* arena) : _arena(arena), _slots(arena, 256) {}

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
  for (auto& slot : _slots) {
    slot = JobGraphNodeSlot(_arena);
  }
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
