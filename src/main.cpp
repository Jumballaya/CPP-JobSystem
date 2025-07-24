#include <iostream>

#include "ArenaVector.hpp"
#include "FrameArena.hpp"
#include "ThreadArenaRegistry.hpp"

struct Entity {
  float x;
  float y;
  float z;
  float velocity;
  float acceleration;
};

int main() {
  FrameArena arena(1024);
  ThreadArenaRegistry::set(&arena);

  ArenaVector<Entity> ents(&arena, 10);

  for (int i = 0; i < 10; ++i) {
    float x = i * 10.0f;
    float y = i * 20.0f;
    float z = i * 30.0f;
    float vel = i / 10.0f;
    float acc = (i * vel);
    ents.emplace_back(x, y, z, vel, acc);
  }

  for (auto& ent : ents) {
    std::cout << "Entity:\nPosition: <" << ent.x << ", " << ent.y << ", " << ent.z << ">\nVelocity: " << ent.velocity << "\nAcceleration: " << ent.acceleration << "\n";
  }

  ThreadArenaRegistry::clear();
}