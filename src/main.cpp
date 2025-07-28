#include <chrono>
#include <iostream>
#include <thread>

#include "JobGraph.hpp"
#include "JobGraphNode.hpp"
#include "JobSystem.hpp"

// Simulated workload with input + output
struct ComputeNode : JobGraphNode<int, int> {
  static void run(const int* input, int* output, FrameArena* arena) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // Simulate CPU work
    *output = (*input) * 2;
    std::cout << "[ComputeNode] Computed: " << *input << " -> " << *output << "\n";
  }
};

// Workload with only input
struct PrintNode : JobGraphNode<std::string> {
  static void run(const std::string* message, FrameArena* arena) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "[PrintNode] Message: " << *message << "\n";
  }
};

// Final node that consumes two inputs
struct CombineNode : JobGraphNode<std::pair<int, int>> {
  static void run(const std::pair<int, int>* in, FrameArena* arena) {
    int result = in->first + in->second;
    std::cout << "[CombineNode] Sum: " << in->first << " + " << in->second << " = " << result << "\n";
  }
};

int main() {
  std::cout << "🧵 Launching JobSystem...\n";

  JobSystem system(std::thread::hardware_concurrency());

  auto graph = system.createGraph(MemoryClass::Frame);

  // Input/output buffers (in real engine, these might live in components or asset jobs)
  int inputA = 10, inputB = 20;
  int outputA = 0, outputB = 0;

  std::string printMsg = "Hello from JobGraph!";

  auto nodeA = graph.addNode<ComputeNode>(&inputA, &outputA);
  auto nodeB = graph.addNode<ComputeNode>(&inputB, &outputB);
  auto nodePrint = graph.addNode<PrintNode>(&printMsg);

  // CombineNode waits on A + B
  auto combineInput = graph.frameArena().allocate<std::pair<int, int>>();
  auto nodeCombine = graph.addNode<CombineNode>(combineInput);

  // Setup dependencies:
  graph.setDependencies(nodeCombine, {nodeA, nodeB});
  graph.setDependencies(nodePrint, {nodeCombine});

  // OnGraphComplete (optional)
  graph.setOnGraphComplete([](GraphNodeHandle handle, void* userData) {
    std::cout << "✅ JobGraph finished for root node: " << handle.index << "\n";
  },
                           nullptr);

  // Submit graph
  system.submitGraph(graph);

  // Wait on root node (or the one you care about)
  JobHandle rootHandle = nodePrint.jobHandle;
  system.wait(rootHandle);

  std::cout << "🏁 All jobs complete.\n";
  return 0;
}