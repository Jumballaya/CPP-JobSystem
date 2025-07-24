#include "JobGraph.hpp"

JobGraph::JobGraph(FrameArena* arena) : _arena(arena) {
  _capacity = 256;
  _slots = _arena->allocate<JobGraphNodeSlot>(_capacity);
  _count = 0;
}

void JobGraph::setDependencies(GraphNodeHandle node, std::initializer_list<GraphNodeHandle> deps) {
  assert(node.index < _count);
  auto& slot = _slots[node.index];
  slot.inDegree = static_cast<uint32_t>(deps.size());

  auto* depStorage = _arena->allocate<GraphNodeHandle>(deps.size());
  std::copy(deps.begin(), deps.end(), depStorage);
  slot.dependents = {depStorage, deps.size()};

  for (auto dep : deps) {
    assert(dep.index < _count);
    JobGraphNodeSlot& depSlot = _slots[dep.index];
    GraphNodeHandle* depDepStorage = _arena->allocate<GraphNodeHandle>(1);
    *depDepStorage = node;
    depSlot.dependents = std::span<GraphNodeHandle>(depDepStorage, 1);
  }
}