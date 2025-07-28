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