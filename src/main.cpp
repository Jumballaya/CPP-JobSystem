#include <iostream>

#include "FrameArena.hpp"
#include "Job.hpp"
#include "JobGraph.hpp"
#include "ThreadArenaRegistry.hpp"

struct AddIntsNode : JobGraphNode<int, int> {
  static void run(const int* input, int* output, FrameArena* arena) {
    *output = *input + 42;
  }
};

struct PrintInt : JobGraphNode<int> {
  static void run(const int* input, FrameArena* arena) {
    std::cout << "[Print Node]: " << *input << "\n";
  }
};

int main() {
  FrameArena arena(1024);
  ThreadArenaRegistry::set(&arena);

  int input = 5;
  int* output = arena.allocate<int>();

  Job job = makeJob<AddIntsNode>(&input, output, &arena);
  Job job2 = makeJob<PrintInt>(output, &arena);

  job.fn(job.userData);
  std::cout << "Job output: " << *output << "\n";

  job2.fn(job2.userData);

  ThreadArenaRegistry::clear();
}