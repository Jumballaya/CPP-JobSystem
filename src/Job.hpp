#pragma once

#include <atomic>
#include <cstdint>

#include "FrameArena.hpp"

enum JobFlags : uint32_t {
  None = 0,
  HighPriority = 1 << 0,    // @TODO: Schedules ahead of normal jobs (used for rendering, audio, user input)
  LongRunning = 1 << 1,     // @TODO: Long running jobs (helps avoid stealing threads for jobs that block (I/O, streaming))
  Cancelable = 1 << 2,      // @TODO: Cancelable jobs (enables early-out for expensive or unneeded jobs)
  FrameLocal = 1 << 3,      // @TODO: Signals the job's memory is frame-bound (tells system that it may safely reset arena after the frame ends)
  WorkerAffinity = 1 << 4,  // @TODO: Job must run on a specific thread (tagged workers)
  Detached = 1 << 5,        // @TODO: Fire and forget job (allows system to pool or reuse resources aggressively)
  DebugTrace = 1 << 6,      // @TODO: Enable logging/profiling for this job only (tracing and debugging)
  SkipArenaReset = 1 << 7   // @TODO: Job system won’t reset thread-local arena after this job (job allocates long-lived memory)
};

enum JobState {
  Pending,
  Running,
  Completed,
  Cancelled
};

struct JobControlBlock {
  std::atomic<JobState> state = JobState::Pending;
  std::atomic<bool> cancelRequested = false;
};

struct JobHandle {
  uint32_t id = 0;
  uint32_t generation = 0;
  JobControlBlock* control = nullptr;

  bool isValid() const {
    return control != nullptr;
  }
};

struct Job {
  using JobFn = void (*)(void*);

  JobFn fn = nullptr;
  void* userData = nullptr;
  FrameArena* arena = nullptr;
  JobControlBlock* control = nullptr;
  JobFlags flags = JobFlags::None;
};

inline bool HasFlag(JobFlags flags, JobFlags flag) {
  return (flags & flag) == flag;
}