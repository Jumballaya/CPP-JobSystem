#pragma once

#include <concepts>
#include <cstdint>

#include "FrameArena.hpp"
#include "Job.hpp"
#include "JobSystem.hpp"

//
//  Always start with a generation of 1 or greater when
//  creating the handle, a generation of 0 signifies an
//  invalid handle.
//
struct GraphNodeHandle {
  uint32_t index = 0;
  uint32_t generation = 0;

  bool isValid() const { return generation != 0; }
  bool operator==(const GraphNodeHandle&) const = default;
};

//
//  Not intended for direct use. Inherit from this class to create the
//  a job node.
//
template <typename In, typename Out = void>
struct JobGraphNode {
  static void run(const In* input, Out* output, FrameArena* arena);
};

//
//  Checks:
//      T has static run() method
//      run takes in (const In*, Out*, FrameArena*)
//      The return type of run is void
//
template <typename T, typename In, typename Out>
concept IsJobGraphNodeWithOutput = requires(const In* in, Out* out, FrameArena* arena) {
  { T::run(in, out, arena) } -> std::same_as<void>;
};

//
//  Checks:
//      T has static run() method
//      run takes in (const In*, FrameArena*)
//      The return type of run is void
//
template <typename T, typename In>
concept IsJobGraphNodeNoOutput = requires(const In* in, FrameArena* arena) {
  { T::run(in, arena) } -> std::same_as<void>;
};

//
//  Unified concept
//
template <typename T, typename In, typename Out = void>
concept IsJobGraphNode = IsJobGraphNodeWithOutput<T, In, Out> || IsJobGraphNodeNoOutput<T, In>;

//
//  Job Creation API
//
template <typename Node, typename In, typename Out = void>
  requires IsJobGraphNode<Node, In, Out>
Job makeJob(const In* input, Out* output, FrameArena* arena, JobSystem* system = nullptr, JobGraph* graph = nullptr, GraphNodeHandle handle = {}, JobFlags flags = JobFlags::None) {
  struct JobData {
    const In* input;
    Out* output;
    FrameArena* arena;
    JobSystem* system;
    JobGraph* graph;
    GraphNodeHandle handle;
  };

  JobData* data = arena->allocate<JobData>();
  data->input = input;
  data->output = output;
  data->arena = arena;
  data->system = system;
  data->graph = graph;
  data->handle = handle;

  return Job{
      .fn = [](void* userData) {
        auto* d = static_cast<JobData*>(userData);
        Node::run(d->input, d->output, d->arena);
        if (d->graph && d->handle.isValid()) {
          d->graph->onJobComplete(d->handle, *d->system);
        }
      },
      .userData = data,
      .arena = arena,
      .control = nullptr,
      .flags = flags};
}

template <typename Node, typename In>
  requires IsJobGraphNode<Node, In>
Job makeJob(const In* input, FrameArena* arena, JobSystem* system = nullptr, JobGraph* graph = nullptr, GraphNodeHandle handle = {}, JobFlags flags = JobFlags::None) {
  struct JobData {
    const In* input;
    FrameArena* arena;
    JobSystem* system;
    JobGraph* graph;
    GraphNodeHandle handle;
  };

  JobData* data = arena->allocate<JobData>();
  data->input = input;
  data->arena = arena;
  data->system = system;
  data->graph = graph;
  data->handle = handle;

  return Job{
      .fn = [](void* userData) {
        auto* d = static_cast<JobData*>(userData);
        Node::run(d->input, d->arena);
        if (d->graph && d->handle.isValid()) {
          d->graph->onJobComplete(d->handle, *d->system);
        }
      },
      .userData = data,
      .arena = arena,
      .control = nullptr,
      .flags = flags};
}