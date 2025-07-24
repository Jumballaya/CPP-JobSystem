#pragma once

#include <cstddef>
#include <thread>
#include <vector>

#include "Job.hpp"

class JobSystem {
 public:
  JobSystem(size_t threadCount);

  bool isComplete(const JobHandle& handle);
  bool isCancelled(const JobHandle& handle);
  bool cancel(JobHandle handle);
  void wait(JobHandle handle);

 private:
  size_t _threadCount;

  std::vector<std::thread> _threads;
};