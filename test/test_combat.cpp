#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/projectile.h"
#include "object/zombie.h"
#include "world.h"
#include <cmath>

using namespace pvz_emulator::object;

class CombatTest {
protected:
  CombatTest() {
    w = std::make_unique<pvz_emulator::world>(scene_type::day);
    w->reset();
    w->scene.stop_spawn = true;
  }

  std::unique_ptr<pvz_emulator::world> w;

  plant *find_plant(plant_type type) {
    for (auto &p : w->scene.plants) {
      if (!p.is_dead && p.type == type)
        return &p;
    }
    return nullptr;
  }

  zombie *find_zombie(zombie_type type) {
    for (auto &z : w->scene.zombies) {
      if (!z.is_dead && z.type == type)
        return &z;
    }
    return nullptr;
  }

  projectile *find_projectile(projectile_type type) {
    for (auto &proj : w->scene.projectiles) {
      if (proj.type == type)
        return &proj;
    }
    return nullptr;
  }
};

TEST_CASE_METHOD(CombatTest, "PeashooterFiringIntervals", "[CombatTest]") {
  w->select_plants({plant_type::pea_shooter});
  w->plant(plant_type::pea_shooter, 2, 1);
  plant *ps = find_plant(plant_type::pea_shooter);
  REQUIRE(ps != nullptr);

  w->zombie_factory.create(zombie_type::zombie, 2);

  // 1. Initial firing delay should be 0 ~ 150
  CHECK(ps->countdown.generate >= 0);
  CHECK(ps->countdown.generate <= 150);

  // 2. Wait for first shot with safety limit
  int safety = 0;
  while (w->scene.projectiles.size() == 0 && safety < 1000) {
    w->update();
    safety++;
  }
  REQUIRE(safety < 1000);

  // 3. Check reset interval
  CHECK(ps->countdown.generate >= 102);
  CHECK(ps->countdown.generate <= 116);

  // 4. Check subsequent interval
  unsigned int last_generate = ps->countdown.generate;
  safety = 0;
  while (w->scene.projectiles.size() == 1 && safety < 1000) {
    w->update();
    safety++;
  }
  REQUIRE(safety < 1000);

  CHECK(ps->countdown.generate >= 102);
  CHECK(ps->countdown.generate <= 116);
}

TEST_CASE_METHOD(CombatTest, "PeashooterDamage", "[CombatTest]") {
  w->select_plants({plant_type::pea_shooter});
  w->plant(plant_type::pea_shooter, 2, 0);

  // Spawn a zombie ahead
  zombie &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = 400.0F;
  z.int_x = 400;
  int initial_hp = z.hp;

  // Simulate until hit
  bool hit = false;
  for (int i = 0; i < 1000; ++i) {
    w->update();
    if (z.hp < initial_hp) {
      hit = true;
      break;
    }
  }

  REQUIRE(hit);
  CHECK(z.hp == initial_hp - 20);
}

TEST_CASE_METHOD(CombatTest, "SnowPeaSlowingEffect", "[CombatTest]") {
  w->select_plants({plant_type::snow_pea});
  w->plant(plant_type::snow_pea, 2, 0);

  zombie &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = 300.0f;
  z.int_x = 300;
  float normal_fps = z.reanim.fps;

  // Simulate until hit
  bool hit = false;
  for (int i = 0; i < 500; ++i) {
    w->update();
    if (z.countdown.slow > 0) {
      hit = true;
      break;
    }
  }

  REQUIRE(hit);
  // When slowed, FPS should be reduced to 50%
  CHECK(std::abs(z.reanim.fps - normal_fps * 0.5F) <= 0.1F);
}