#include "catch_amalgamated.hpp"
#include "world.h"
#include <memory>

using namespace pvz_emulator;
using namespace pvz_emulator::object;

TEST_CASE("InitialSunDropIntervalDay", "[SkySunDrop]") {
  auto w = std::make_unique<world>(scene_type::day);
  w->reset();

  unsigned int initial_countdown = w->scene.sun.natural_sun_countdown;
  CHECK(initial_countdown >= 425);
  CHECK(initial_countdown <= 699);
}

TEST_CASE("FollowingSunDropIntervalFormula", "[SkySunDrop]") {
  auto w = std::make_unique<world>(scene_type::day);
  w->reset();
  w->scene.sun.sun = 0;
  w->scene.stop_spawn = true;

  for (int n = 0; n < 3; ++n) {
    unsigned int next_n = w->scene.sun.natural_sun_generated + 1;
    unsigned int min_interval = std::min(950U, (next_n * 10) + 425);
    unsigned int max_interval = min_interval + 274;

    while (w->scene.sun.natural_sun_countdown > 1) {
      w->update();
    }
    w->update();

    unsigned int new_countdown = w->scene.sun.natural_sun_countdown;
    CHECK(new_countdown >= min_interval);
    CHECK(new_countdown <= max_interval);
  }
}

TEST_CASE("StabilityAfterManyDrops", "[SkySunDrop]") {
  auto w = std::make_unique<world>(scene_type::day);
  w->reset();

  w->scene.sun.natural_sun_generated = 53;
  w->scene.sun.natural_sun_countdown = 1;
  w->update();

  unsigned int new_countdown = w->scene.sun.natural_sun_countdown;
  CHECK(new_countdown >= 950);
  CHECK(new_countdown <= 1224);
}

TEST_CASE("NoSkySunDropAtNight", "[SkySunDrop]") {
  auto w_night = std::make_unique<world>(scene_type::night);
  w_night->reset();
  w_night->scene.stop_spawn = true;

  unsigned int initial_sun = w_night->scene.sun.sun;
  for (int i = 0; i < 2000; ++i) {
    w_night->update();
  }

  CHECK(initial_sun == w_night->scene.sun.sun);
  CHECK(w_night->scene.sun.natural_sun_generated == 0);
}