#include "catch_amalgamated.hpp"
#include "object/griditem.h"
#include "object/plant.h"
#include "object/projectile.h"
#include "object/zombie.h"
#include "world.h"
#include <cmath>

using namespace pvz_emulator::object;

class MushroomSpecialTest {
protected:
  MushroomSpecialTest() {
    w = std::make_unique<pvz_emulator::world>(scene_type::night);
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

  griditem *find_griditem(griditem_type type) {
    for (auto &item : w->scene.griditems) {
      if (item.type == type)
        return &item;
    }
    return nullptr;
  }
};

TEST_CASE_METHOD(MushroomSpecialTest, "FumeshroomMultiPenetration",
                 "[MushroomSpecialTest]") {
  w->select_plants({plant_type::fumeshroom});
  REQUIRE(w->plant(plant_type::fumeshroom, 2, 0));
  plant *p = find_plant(plant_type::fumeshroom);
  REQUIRE(p != nullptr);

  zombie &z1 = w->zombie_factory.create(zombie_type::zombie, 2);
  z1.x = 200.0F;
  z1.int_x = 200;
  zombie &z2 = w->zombie_factory.create(zombie_type::zombie, 2);
  z2.x = 220.0F;
  z2.int_x = 220;
  zombie &z3 = w->zombie_factory.create(zombie_type::zombie, 2);
  z3.x = 240.0F;
  z3.int_x = 240;

  int initial_hp = 270;
  bool all_hit = false;
  for (int i = 0; i < 200; ++i) {
    w->update();
    if (z1.hp < initial_hp && z2.hp < initial_hp && z3.hp < initial_hp) {
      all_hit = true;
      break;
    }
  }

  REQUIRE(all_hit);
  CHECK(z1.hp == initial_hp - 20);
  CHECK(z2.hp == initial_hp - 20);
  CHECK(z3.hp == initial_hp - 20);
}

TEST_CASE_METHOD(MushroomSpecialTest, "FumeshroomScreenDoorBypass",
                 "[MushroomSpecialTest]") {
  w->select_plants({plant_type::fumeshroom});
  REQUIRE(w->plant(plant_type::fumeshroom, 1, 0));

  zombie &z = w->zombie_factory.create(zombie_type::screendoor, 1);
  z.x = 200.0F;
  z.int_x = 200;
  int initial_hp = z.hp;
  int initial_door_hp = z.accessory_2.hp;

  bool hit = false;
  for (int i = 0; i < 200; ++i) {
    w->update();
    if (z.hp < initial_hp) {
      hit = true;
      break;
    }
  }

  REQUIRE(hit);
  CHECK(z.hp == initial_hp - 20);
  CHECK(z.accessory_2.hp == initial_door_hp - 20);
}

TEST_CASE_METHOD(MushroomSpecialTest, "HypnoshroomFullCharmLogic",
                 "[MushroomSpecialTest]") {
  w->plant_factory.create(plant_type::hypnoshroom, 2, 5);
  w->plant_factory.create(plant_type::pea_shooter, 2, 1);

  zombie &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = 455.0F;
  z.int_x = 455;

  int ticks = 0;
  while (!z.is_hypno && ticks < 500) {
    w->update();
    ticks++;
  }
  REQUIRE(z.is_hypno);

  for (int i = 0; i < 150; ++i)
    w->update();
  w->scene.projectiles.clear();
  for (int i = 0; i < 100; ++i)
    w->update();

  INFO("Plants should not attack charmed zombies");
  CHECK(w->scene.projectiles.size() == 0);
}

TEST_CASE_METHOD(MushroomSpecialTest, "HypnoDiggerZombie",
                 "[MushroomSpecialTest]") {
  w->select_plants({plant_type::hypnoshroom});
  w->plant(plant_type::hypnoshroom, 2, 3);

  zombie &digger = w->zombie_factory.create(zombie_type::digger, 2);
  digger.status = zombie_status::digger_walk_left;
  digger.has_item_or_walk_left = true;
  digger.x = 280.0F;
  digger.int_x = 280;

  w->zombie.reanim.set(digger, zombie_reanim_name::anim_walk,
                       reanim_type::repeat, 0);

  int ticks = 0;
  while (!digger.is_hypno && ticks < 1000) {
    w->update();
    ticks++;
  }

  REQUIRE(digger.is_hypno);
  CHECK(digger.is_walk_right());
  CHECK(std::abs(digger.dx - 0.17F) <= 0.05F);
}

TEST_CASE_METHOD(MushroomSpecialTest, "HypnoYetiZombie",
                 "[MushroomSpecialTest]") {
  w->select_plants({plant_type::hypnoshroom});
  w->plant(plant_type::hypnoshroom, 2, 5);

  zombie &yeti = w->zombie_factory.create(zombie_type::yeti, 2);
  yeti.x = 455.0F;
  yeti.int_x = 455;

  int ticks = 0;
  while (!yeti.is_hypno && ticks < 1000) {
    w->update();
    ticks++;
  }

  REQUIRE(yeti.is_hypno);
  CHECK(yeti.is_walk_right());
  CHECK(std::abs(yeti.dx - 0.17F) <= 0.05F);
}

TEST_CASE_METHOD(MushroomSpecialTest, "HypnoshroomImmuneGargantuar_Repeated",
                 "[MushroomSpecialTest]") {
  w->select_plants({plant_type::hypnoshroom});
  w->plant(plant_type::hypnoshroom, 2, 5);
  plant *hypno = find_plant(plant_type::hypnoshroom);

  zombie &g = w->zombie_factory.create(zombie_type::gargantuar, 2);
  g.x = 480.0F;
  g.int_x = 480;

  bool eaten_or_dead = false;
  for (int i = 0; i < 600; ++i) {
    w->update();
    if (hypno->is_dead) {
      eaten_or_dead = true;
      break;
    }
  }

  CHECK(eaten_or_dead);
  INFO("Gargantuar should crush Hypnoshroom and not be charmed");
  CHECK_FALSE(g.is_hypno);
}

TEST_CASE_METHOD(MushroomSpecialTest, "GraveBusterEatsGrave",
                 "[MushroomSpecialTest]") {
  w->griditem_factory.create(griditem_type::grave, 2, 5);
  w->select_plants({plant_type::grave_buster});
  REQUIRE(w->plant(plant_type::grave_buster, 2, 5));

  plant *buster = find_plant(plant_type::grave_buster);
  REQUIRE(buster != nullptr);
  CHECK(buster->status == plant_status::grave_buster_land);

  int safety_counter = 0;
  while (buster->status == plant_status::grave_buster_land &&
         safety_counter < 500) {
    w->update();
    safety_counter++;
  }

  REQUIRE(safety_counter < 500);
  CHECK(buster->status == plant_status::grave_buster_idle);

  bool grave_destroyed = false;
  for (int i = 0; i < 600; ++i) {
    w->update();
    if (buster->is_dead) {
      grave_destroyed = true;
      break;
    }
  }

  CHECK(grave_destroyed);
  CHECK(find_griditem(griditem_type::grave) == nullptr);
}