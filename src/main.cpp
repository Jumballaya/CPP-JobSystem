#include <iostream>

#include "FrameArena.hpp"
#include "Job.hpp"
#include "ThreadArenaRegistry.hpp"

struct PrintData {
  const char* label;
  int count;
};

void printLabel(void* data) {
  auto* pd = static_cast<PrintData*>(data);
  std::cout << "[Job] " << pd->label << ": " << pd->count << "\n";
}

int main() {
  FrameArena arena(1024);
  ThreadArenaRegistry::set(&arena);

  PrintData* data = arena.allocate<PrintData>();
  data->label = "Apples";
  data->count = 5;

  JobControlBlock control;

  Job job = {
      .fn = &printLabel,
      .userData = data,
      .arena = &arena,
      .control = &control,
      .flags = JobFlags::None};

  job.control->state = JobState::Running;
  job.fn(job.userData);
  job.control->state = JobState::Completed;

  ThreadArenaRegistry::clear();
}