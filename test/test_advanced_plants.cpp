#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/projectile.h"
#include "object/zombie.h"
#include "system/plant/plant.h"
#include "world.h"

using namespace pvz_emulator::object;

class AdvancedPlantsTest {
protected:
  AdvancedPlantsTest() {
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
};

TEST_CASE_METHOD(AdvancedPlantsTest, "ChomperBiteAndChew",
                 "[AdvancedPlantsTest]") {
  w->select_plants({plant_type::chomper});
  REQUIRE(w->plant(plant_type::chomper, 2, 1));

  plant *chomper = find_plant(plant_type::chomper);
  REQUIRE(chomper != nullptr);
  CHECK(chomper->status == plant_status::wait);

  // Spawn a zombie in range
  zombie &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = chomper->x + 40.0F; // Close enough to bite
  z.int_x = (int)z.x;

  // 1. Wait for bite begin
  int safety = 0;
  while (chomper->status == plant_status::wait && safety < 100) {
    w->update();
    safety++;
  }
  CHECK(chomper->status == plant_status::chomper_bite_begin);

  // 2. Wait for bite execution (70 ticks)
  safety = 0;
  while (chomper->status == plant_status::chomper_bite_begin && safety < 100) {
    w->update();
    safety++;
  }

  // 3. Wait for bite success animation to finish and transition to chew
  safety = 0;
  while (chomper->status == plant_status::chomper_bite_success &&
         safety < 100) {
    w->update();
    safety++;
  }

  // After bite begin, it should destroy the zombie and move to success then
  // chew
  CHECK(z.is_dead == true);
  CHECK(chomper->status == plant_status::chomper_chew);
  CHECK(chomper->countdown.status == 4000);
}

TEST_CASE_METHOD(AdvancedPlantsTest, "ChomperVsGargantuarBiteDamage",
                 "[AdvancedPlantsTest]") {
  w->select_plants({plant_type::chomper});
  REQUIRE(w->plant(plant_type::chomper, 2, 1));

  plant *chomper = find_plant(plant_type::chomper);
  REQUIRE(chomper != nullptr);

  zombie &g = w->zombie_factory.create(zombie_type::gargantuar, 2);
  g.x = chomper->x + 40.0F;
  g.int_x = (int)g.x;
  int initial_hp = g.hp;

  // Simulate until bite happens
  int safety = 0;
  while (chomper->status != plant_status::chomper_bite_fail && safety < 500) {
    w->update();
    safety++;
  }

  CHECK(chomper->status == plant_status::chomper_bite_fail);
  CHECK(g.hp == initial_hp - 40);
  CHECK(g.is_dead == false);
}

TEST_CASE_METHOD(AdvancedPlantsTest, "BloverBlowsBalloon",
                 "[AdvancedPlantsTest]") {
  zombie &b = w->zombie_factory.create(zombie_type::balloon, 2);
  b.status = zombie_status::balloon_flying;

  w->select_plants({plant_type::blover});
  REQUIRE(w->plant(plant_type::blover, 2, 2));

  plant *blover = find_plant(plant_type::blover);
  REQUIRE(blover != nullptr);

  // Simulate until blover activates (50 ticks)
  for (int i = 0; i < 60; ++i) {
    w->update();
    if (b.is_blown)
      break;
  }
  CHECK(b.is_blown);
}

TEST_CASE_METHOD(AdvancedPlantsTest, "CactusPopsBalloon",
                 "[AdvancedPlantsTest]") {
  w->select_plants({plant_type::cactus});
  REQUIRE(w->plant(plant_type::cactus, 2, 1));

  plant *cactus = find_plant(plant_type::cactus);
  REQUIRE(cactus != nullptr);

  zombie &b = w->zombie_factory.create(zombie_type::balloon, 2);
  b.status = zombie_status::balloon_flying;
  b.x = 600.0F;
  b.int_x = 600;
  b.has_balloon = true;

  // Simulate until cactus grows and pops
  int safety = 0;
  while (b.has_balloon && safety < 1000) {
    w->update();
    safety++;
  }
  REQUIRE(safety < 1000);
  CHECK_FALSE(b.has_balloon);
  CHECK(b.status == zombie_status::balloon_falling);
}

TEST_CASE_METHOD(AdvancedPlantsTest, "CobCannonStateAndLaunch",
                 "[AdvancedPlantsTest]") {
  w->select_plants({plant_type::kernelpult, plant_type::cob_cannon});
  w->scene.sun.sun = 2000;
  REQUIRE(w->plant(plant_type::kernelpult, 2, 1));
  w->scene.cards[0].cold_down = 0;
  REQUIRE(w->plant(plant_type::kernelpult, 2, 2));

  REQUIRE(w->plant(plant_type::cob_cannon, 2, 1));
  plant *cob = find_plant(plant_type::cob_cannon);
  REQUIRE(cob != nullptr);
  CHECK(cob->status == plant_status::cob_cannon_unarmed_idle);

  // 1. Force charge (default 500 ticks)
  cob->countdown.status = 0;
  w->update();
  CHECK(cob->status == plant_status::cob_cannon_charge);

  // 2. Wait for armed
  int safety = 0;
  while (cob->status == plant_status::cob_cannon_charge && safety < 1000) {
    w->update();
    safety++;
  }
  REQUIRE(safety < 1000);
  CHECK(cob->status == plant_status::cob_cannon_armed_idle);

  // 3. Launch
  pvz_emulator::system::plant_cob_cannon cob_subsystem(w->scene);
  bool success = cob_subsystem.launch(*cob, 500, 200);
  CHECK(success);
  CHECK(cob->status == plant_status::cob_cannon_launch);
  CHECK(cob->countdown.launch == 206);
}