#include "catch_amalgamated.hpp"
#include "world.h"
#include <chrono>
#include <iostream>

using namespace pvz_emulator;
using namespace pvz_emulator::object;

TEST_CASE("StressCollisionWorstCase", "[PerformanceTest]") {
  world w(scene_type::pool);
  w.scene.stop_spawn = true;
  w.scene.sun.sun = 99999;

  for (int r = 0; r < 6; ++r) {
    for (int c = 0; c < 7; ++c) {
      w.plant_factory.create(plant_type::pea_shooter, r, c);
    }
  }

  for (int i = 0; i < 500; ++i) {
    auto &z = w.zombie_factory.create(zombie_type::zombie, i % 6);
    z.x = 200.0F + (i * 5);
  }

  for (int i = 0; i < 10; ++i)
    w.update();

  std::cout << "Projectiles on screen: " << w.scene.projectiles.size() << '\n';
  std::cout << "Zombies on screen: " << w.scene.zombies.size() << '\n';

  auto start = std::chrono::high_resolution_clock::now();
  const int TEST_FRAMES = 10000;
  for (int i = 0; i < TEST_FRAMES; ++i) {
    w.update();
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end - start;
}