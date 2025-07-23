cmake_minimum_required(VERSION 3.16)
project(spsc_queue)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(spsc
  src/main.cpp
  src/ThreadPool.hpp
  src/ThreadPool.cpp
  src/FrameArena.hpp
  src/FrameArena.cpp
  src/ThreadArenaRegistry.hpp
  src/ThreadArenaRegistry.cpp
  src/Job.hpp
)