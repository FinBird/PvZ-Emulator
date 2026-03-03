#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/zombie.h"
#include "world.h"

using namespace pvz_emulator::object;

class SpecialMechanicsTest {
protected:
  SpecialMechanicsTest() {
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

TEST_CASE_METHOD(SpecialMechanicsTest, "ImitaterTransformation",
                 "[SpecialMechanicsTest]") {
  w->scene.cards[0].type = plant_type::imitater;
  w->scene.cards[0].imitater_type = plant_type::pea_shooter;
  w->scene.cards[0].cold_down = 0;

  REQUIRE(w->plant(0, 2, 2));

  plant *imitater = find_plant(plant_type::imitater);
  REQUIRE(imitater != nullptr);

  imitater->countdown.status = 0;
  w->update();
  CHECK(imitater->status == plant_status::imitater_morphing);

  int ticks = 0;
  while (!imitater->is_dead && ticks < 500) {
    w->update();
    ticks++;
  }

  CHECK(imitater->is_dead);

  plant *new_plant = nullptr;
  for (auto &p : w->scene.plants) {
    if (!p.is_dead && p.row == 2 && p.col == 2 &&
        p.type != plant_type::imitater) {
      new_plant = &p;
      break;
    }
  }
  REQUIRE(new_plant != nullptr);
  CHECK(new_plant->type == plant_type::pea_shooter);
}

TEST_CASE_METHOD(SpecialMechanicsTest, "UmbrellaLeafBlockBungee",
                 "[SpecialMechanicsTest]") {
  w->select_plants({plant_type::umbrella_leaf});
  REQUIRE(w->plant(plant_type::umbrella_leaf, 2, 2));

  plant *umbrella = find_plant(plant_type::umbrella_leaf);
  REQUIRE(umbrella != nullptr);

  zombie &z = w->zombie_factory.create(zombie_type::bungee, 2);
  z.bungee_col = 2;
  z.x = umbrella->x;
  z.int_x = (int)z.x;
  z.y = umbrella->y;
  z.int_y = (int)z.y;
  z.status = zombie_status::bungee_target_drop;
  z.dy = 45.0F;

  w->update();

  CHECK(umbrella->status == plant_status::umbrella_leaf_block);
}

TEST_CASE_METHOD(SpecialMechanicsTest, "UmbrellaLeafBlockBasketball",
                 "[SpecialMechanicsTest]") {
  w->select_plants({plant_type::umbrella_leaf});
  REQUIRE(w->plant(plant_type::umbrella_leaf, 2, 2));

  plant *umbrella = find_plant(plant_type::umbrella_leaf);
  REQUIRE(umbrella != nullptr);

  zombie &catapult = w->zombie_factory.create(zombie_type::catapult, 2);
  catapult.x = 600.0F;
  catapult.int_x = 600;

  bool blocked = false;
  int ticks = 0;

  while (ticks < 1000) {
    w->update();
    if (umbrella->status == plant_status::umbrella_leaf_block) {
      blocked = true;
      break;
    }
    ticks++;
  }

  INFO("Umbrella leaf failed to block the basketball within 1000 ticks.");
  CHECK(blocked);
  CHECK(umbrella->status == plant_status::umbrella_leaf_block);
}

TEST_CASE("DisableBootDelayEffect", "[MechanicsConfig]") {
  pvz_emulator::world w(scene_type::day);
  w.reset();

  w.scene.disable_plant_boot_delay = false;
  w.select_plants({plant_type::pea_shooter});
  w.plant(plant_type::pea_shooter, 2, 1);
  plant *p1 = nullptr;
  for (auto &p : w.scene.plants)
    p1 = &p;
  REQUIRE(p1 != nullptr);

  w.reset();
  w.scene.disable_plant_boot_delay = true;
  w.select_plants({plant_type::pea_shooter, plant_type::sunflower});

  w.plant(plant_type::pea_shooter, 2, 1);
  plant *ps = nullptr;
  for (auto &p : w.scene.plants)
    if (p.type == plant_type::pea_shooter)
      ps = &p;
  CHECK(ps->countdown.generate == 0);

  w.plant(plant_type::sunflower, 3, 1);
  plant *sf = nullptr;
  for (auto &p : w.scene.plants)
    if (p.type == plant_type::sunflower)
      sf = &p;
  CHECK(sf->countdown.generate == 0);
}

TEST_CASE("InstantAttackWithNoDelay", "[MechanicsConfig]") {
  pvz_emulator::world w(scene_type::day);
  w.reset();
  w.scene.stop_spawn = true;
  w.scene.disable_plant_boot_delay = true;

  w.select_plants({plant_type::pea_shooter});
  w.plant(plant_type::pea_shooter, 2, 0);

  auto &z = w.zombie_factory.create(zombie_type::zombie, 2);
  z.x = 700.0F;
  z.int_x = 700;

  w.update();

  plant *ps = nullptr;
  for (auto &p : w.scene.plants) {
    if (!p.is_dead && p.type == plant_type::pea_shooter) {
      ps = &p;
      break;
    }
  }

  REQUIRE(ps != nullptr);
  INFO("Peashooter should have started its launch countdown (35) immediately.");
  CHECK(ps->countdown.launch > 0);
  CHECK(ps->countdown.launch == 35);
}