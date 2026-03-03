#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/zombie.h"
#include "world.h"

using namespace pvz_emulator;
using namespace pvz_emulator::object;

class FumeShroomTest {
protected:
  FumeShroomTest() {
    w = std::make_unique<world>(scene_type::night);
    w->reset();
    w->scene.stop_spawn = true;
  }

  std::unique_ptr<world> w;
};

TEST_CASE_METHOD(FumeShroomTest, "MultiPenetration", "[FumeShroomTest]") {
  w->select_plants({plant_type::fumeshroom});
  w->plant(plant_type::fumeshroom, 2, 0);

  auto &z1 = w->zombie_factory.create(zombie_type::zombie, 2);
  z1.x = 200.0F;
  auto &z2 = w->zombie_factory.create(zombie_type::zombie, 2);
  z2.x = 250.0F;
  auto &z3 = w->zombie_factory.create(zombie_type::zombie, 2);
  z3.x = 300.0F;

  for (int i = 0; i < 200; ++i)
    w->update();

  CHECK(z1.hp < 270);
  CHECK(z2.hp < 270);
  CHECK(z3.hp < 270);
}

TEST_CASE_METHOD(FumeShroomTest, "BypassesScreenDoor", "[FumeShroomTest]") {
  w->select_plants({plant_type::fumeshroom});
  w->plant(plant_type::fumeshroom, 2, 0);

  auto &z = w->zombie_factory.create(zombie_type::screendoor, 2);
  z.x = 200.0F;
  int initial_door_hp = z.accessory_2.hp;
  int initial_body_hp = z.hp;

  for (int i = 0; i < 200; ++i)
    w->update();

  CHECK(z.accessory_2.hp < initial_door_hp);
  CHECK(z.hp < initial_body_hp);
}