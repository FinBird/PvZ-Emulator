#include "catch_amalgamated.hpp"
#include "world.h"
#include <memory>

using namespace pvz_emulator;
using namespace pvz_emulator::object;

TEST_CASE("InitialCooldown", "[SunPlants]") {
  auto w = std::make_unique<world>(scene_type::day);
  w->reset();
  w->select_plants({plant_type::sunflower});
  w->scene.sun.sun = 9990;
  w->plant(plant_type::sunflower, 1, 1);

  plant *sf = nullptr;
  for (auto &p : w->scene.plants) {
    if (!p.is_dead && p.type == plant_type::sunflower) {
      sf = &p;
      break;
    }
  }
  REQUIRE(sf != nullptr);
  CHECK(sf->countdown.generate >= 300);
  CHECK(sf->countdown.generate <= 1250);
}

TEST_CASE("SunflowerProductionInterval", "[SunPlants]") {
  auto w = std::make_unique<world>(scene_type::day);
  w->reset();
  w->scene.sun.natural_sun_countdown = 99999;
  w->scene.stop_spawn = true;
  w->select_plants({plant_type::sunflower});
  w->plant(plant_type::sunflower, 1, 1);

  plant *sf = nullptr;
  for (auto &p : w->scene.plants) {
    if (!p.is_dead && p.type == plant_type::sunflower) {
      sf = &p;
      break;
    }
  }
  REQUIRE(sf != nullptr);

  while (sf->countdown.generate > 1) {
    w->update();
  }
  w->update();
  CHECK(sf->countdown.generate >= 2350);
  CHECK(sf->countdown.generate <= 2500);
}

TEST_CASE("TwinSunflowerProduction", "[SunPlants]") {
  auto w = std::make_unique<world>(scene_type::day);
  w->reset();
  w->scene.sun.sun = 1000;
  w->scene.sun.natural_sun_countdown = 99999;
  w->select_plants({plant_type::sunflower, plant_type::twin_sunflower});
  w->scene.sun.sun = 1000;
  w->plant(plant_type::sunflower, 1, 1);
  w->scene.cards[0].cold_down = 0;
  w->plant(plant_type::twin_sunflower, 1, 1);

  plant *tsf = nullptr;
  for (auto &p : w->scene.plants) {
    if (!p.is_dead && p.type == plant_type::twin_sunflower) {
      tsf = &p;
      break;
    }
  }
  REQUIRE(tsf != nullptr);

  w->scene.sun.sun = 0;
  while (tsf->countdown.generate > 1) {
    w->update();
  }
  w->update();
  CHECK(w->scene.sun.sun == 50);
}

TEST_CASE("SunshroomGrowthAndProduction", "[SunPlants]") {
  auto w = std::make_unique<world>(scene_type::night);
  w->reset();
  w->scene.stop_spawn = true;
  w->select_plants({plant_type::sunshroom});
  w->plant(plant_type::sunshroom, 1, 1);

  plant *ss = nullptr;
  for (auto &p : w->scene.plants) {
    if (!p.is_dead && p.type == plant_type::sunshroom) {
      ss = &p;
      break;
    }
  }
  REQUIRE(ss != nullptr);

  // 1. Small production
  w->scene.sun.sun = 0;
  while (ss->countdown.generate > 1) {
    w->update();
  }
  w->update();
  CHECK(w->scene.sun.sun == 15);

  // 2. Growth
  ss->countdown.status = 1;
  w->update();
  CHECK(ss->status == plant_status::sunshroom_grow);
  ss->reanim.n_repeated = 1;
  w->update();
  CHECK(ss->status == plant_status::sunshroom_big);

  // 3. Big production
  w->scene.sun.sun = 0;
  while (ss->countdown.generate > 1) {
    w->update();
  }
  w->update();
  CHECK(w->scene.sun.sun == 25);
}

TEST_CASE("SleepAndWake", "[SunPlants]") {
  auto w = std::make_unique<world>(scene_type::day);
  w->reset();
  w->scene.stop_spawn = true;
  w->select_plants({plant_type::sunshroom, plant_type::coffee_bean});
  w->plant(plant_type::sunshroom, 1, 1);

  plant *ss = nullptr;
  for (auto &p : w->scene.plants) {
    if (!p.is_dead && p.type == plant_type::sunshroom) {
      ss = &p;
      break;
    }
  }
  REQUIRE(ss != nullptr);
  CHECK(ss->is_sleeping);

  unsigned int initial_countdown = ss->countdown.generate;
  w->update();
  CHECK(ss->countdown.generate == (int)initial_countdown);

  w->plant(plant_type::coffee_bean, 1, 1);
  for (int i = 0; i < 200; ++i)
    w->update();
  CHECK_FALSE(ss->is_sleeping);

  w->update();
  CHECK(ss->countdown.generate < (int)initial_countdown);
}