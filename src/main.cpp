#include <iostream>

#include "ThreadPool.hpp"

int main() {
  ThreadPool pool(16);

  int i = 0;
  while (true) {
    if (i > 500) break;

    for (int j = 0; j < 16; ++j) {
      pool.submit([i, j]() {
        std::cout << "[Task <" << i << ", " << j << "> ] Running on thread " << std::this_thread::get_id() << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      });
    }

    pool.waitAll();

    ++i;
  }

  std::cout << "[Main] Exiting\n";
}