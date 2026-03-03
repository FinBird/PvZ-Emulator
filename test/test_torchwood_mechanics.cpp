#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/projectile.h"
#include "object/zombie.h"
#include "system/debuff.h"
#include "system/projectile/projectile_factory.h"
#include "world.h"
#include <memory>

using namespace pvz_emulator;
using namespace pvz_emulator::object;

class TorchwoodMechanicsTest {
protected:
  std::unique_ptr<world> w;

  TorchwoodMechanicsTest() {
    w = std::make_unique<world>(scene_type::day);
    w->reset();
    w->scene.stop_spawn = true;
  }
};

TEST_CASE_METHOD(TorchwoodMechanicsTest, "NormalPeaToFirePea",
                 "[TorchwoodMechanicsTest]") {
  w->plant_factory.create(plant_type::pea_shooter, 2, 0);
  w->plant_factory.create(plant_type::torchwood, 2, 1);

  auto &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = 600.0F;
  z.int_x = 600;

  for (int i = 0; i < 500; ++i) {
    w->update();
    if (z.hp < 270)
      break;
  }
  CHECK(z.hp == 230);
}

TEST_CASE_METHOD(TorchwoodMechanicsTest, "SnowPeaToNormalPea",
                 "[TorchwoodMechanicsTest]") {
  w->plant_factory.create(plant_type::snow_pea, 2, 0);
  w->plant_factory.create(plant_type::torchwood, 2, 1);

  auto &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = 600.0F;
  z.int_x = 600;

  for (int i = 0; i < 500; ++i) {
    w->update();
    if (z.hp < 270)
      break;
  }
  CHECK(z.hp == 250);
  INFO("Snow Pea must lose slow effect when passing Torchwood.");
  CHECK(z.countdown.slow == 0);
}

TEST_CASE_METHOD(TorchwoodMechanicsTest, "SplitPeaReverseProjectile",
                 "[TorchwoodMechanicsTest]") {
  w->plant_factory.create(plant_type::torchwood, 2, 2);
  auto &pf = w->projectile_factory;
  auto &proj = pf.create(projectile_type::pea, 2, 350, 247);
  proj.flags = static_cast<unsigned int>(attack_flags::ground);
  proj.motion_type = projectile_motion_type::left_straight;
  proj.dx = -3.33F;

  auto &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = 50.0F;
  z.int_x = 50;

  for (int i = 0; i < 500; ++i) {
    w->update();
    if (z.hp < 270)
      break;
  }
  CHECK(z.hp == 230);
}

TEST_CASE_METHOD(TorchwoodMechanicsTest, "FirePeaSplashUnfreeze",
                 "[TorchwoodMechanicsTest]") {
  auto &pf = w->projectile_factory;
  auto &proj = pf.create(projectile_type::fire_pea, 2, 100, 247);
  proj.flags = static_cast<unsigned int>(attack_flags::ground);
  proj.dx = 3.33F;

  auto &z1 = w->zombie_factory.create(zombie_type::zombie, 2);
  z1.x = 400.0F;
  z1.int_x = 400;

  auto &z2 = w->zombie_factory.create(zombie_type::zombie, 2);
  z2.x = 415.0F;
  z2.int_x = 415;

  w->debuff.set_slowed(z2, 1000);

  for (int i = 0; i < 500; ++i) {
    w->update();
    if (z1.hp < 270)
      break;
  }
  CHECK(z2.hp == 257);
  INFO("Splash damage from Fire Pea must unfreeze the zombie.");
  CHECK(z2.countdown.slow == 0);
}