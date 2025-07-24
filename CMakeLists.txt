cmake_minimum_required(VERSION 3.16)
project(job_systems)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


add_executable(job_demo
  src/main.cpp
  
  src/ArenaVector.hpp
  src/FrameArena.hpp
  src/FrameArena.cpp
  src/Job.hpp
  src/JobGraph.hpp
  src/JobGraph.cpp
  src/JobGraphNode.hpp
  src/JobSystem.hpp
  src/ThreadArenaRegistry.hpp
  src/ThreadArenaRegistry.cpp
)