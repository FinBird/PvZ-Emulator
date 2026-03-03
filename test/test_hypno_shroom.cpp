#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/zombie.h"
#include "world.h"
#include <memory>

using namespace pvz_emulator;
using namespace pvz_emulator::object;

class HypnoShroomTest {
protected:
  std::unique_ptr<world> w;

  HypnoShroomTest() {
    w = std::make_unique<world>(scene_type::night);
    w->reset();
    w->scene.stop_spawn = true;
  }
};

TEST_CASE_METHOD(HypnoShroomTest, "CharmBasicLogicAndPlantImmunity",
                 "[HypnoShroomTest]") {
  w->plant_factory.create(plant_type::hypnoshroom, 2, 5);
  w->plant_factory.create(plant_type::pea_shooter, 2, 1);
  auto &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = 455.0F;
  z.int_x = 455;

  int ticks = 0;
  while (!z.is_hypno && ticks < 1000) {
    w->update();
    ticks++;
  }

  CHECK(z.is_hypno);
  CHECK(z.is_walk_right());

  for (int i = 0; i < 150; ++i)
    w->update();
  w->scene.projectiles.clear();

  for (int i = 0; i < 100; ++i)
    w->update();

  INFO("Plants should not attack charmed zombies");
  CHECK(w->scene.projectiles.size() == 0);
}

TEST_CASE_METHOD(HypnoShroomTest, "SpecialZombiesCharm", "[HypnoShroomTest]") {
  w->plant_factory.create(plant_type::hypnoshroom, 1, 5);
  w->plant_factory.create(plant_type::hypnoshroom, 3, 5);

  auto &digger = w->zombie_factory.create(zombie_type::digger, 1);
  digger.x = 455.0F;
  digger.int_x = 455;
  digger.status = zombie_status::digger_walk_left;
  digger.has_item_or_walk_left = true;
  w->zombie.reanim.set(digger, zombie_reanim_name::anim_walk,
                       reanim_type::repeat, 0);

  auto &yeti = w->zombie_factory.create(zombie_type::yeti, 3);
  yeti.x = 455.0F;
  yeti.int_x = 455;
  yeti.has_item_or_walk_left = true;

  int ticks = 0;
  while ((!digger.is_hypno || !yeti.is_hypno) && ticks < 2000) {
    w->update();
    ticks++;
  }

  CHECK(digger.is_walk_right());
  CHECK(yeti.is_walk_right());
}