#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/zombie.h"
#include "system/util.h"
#include "world.h"
#include <vector>

using namespace pvz_emulator::object;
using namespace pvz_emulator::system;

class CherryBombTest {
protected:
  CherryBombTest() {
    w = std::make_unique<pvz_emulator::world>(scene_type::pool);
    w->reset();
    w->scene.stop_spawn = true;
  }

  std::unique_ptr<pvz_emulator::world> w;

  void wait_for_explosion() {
    // Cherry Bomb countdown.effect is 100
    for (int i = 0; i < 110; ++i) {
      w->update();
    }
  }

  static bool is_zombie_dead(zombie &z) {
    return z.is_dead || z.hp <= 0 ||
           z.status == zombie_status::dying_from_instant_kill ||
           z.status == zombie_status::dying;
  }

  void prepare_plant() {
    w->scene.sun.sun = 1000;
    w->select_plants({plant_type::lily_pad, plant_type::cherry_bomb});
    w->scene.cards[0].cold_down = 0;
    w->scene.cards[1].cold_down = 0;
  }

  static int get_explosion_x(int col) { return (col * 80) + 80; }

  static void center_zombie(zombie &z, int tx, int ty) {
    rect hr;
    z.get_hit_box(hr);

    int ox = hr.x - z.int_x;
    int cx = ox + (hr.width / 2);
    z.x = static_cast<float>(tx - cx);
    z.int_x = tx - cx;

    int oy = hr.y - z.int_y;
    int cy = oy + (hr.height / 2);
    z.y = static_cast<float>(ty - cy);
    z.int_y = ty - cy;

    z.dx = 0;
    z.dy = 0;
  }
};

TEST_CASE_METHOD(CherryBombTest, "CherryBombGargantuarKillCount",
                 "[CherryBomb]") {
  w->reset();
  w->scene.stop_spawn = true;
  zombie &g = w->zombie_factory.create(zombie_type::gargantuar, 1);

  int ex = get_explosion_x(4);
  int ey = get_y_by_row_and_col(w->scene.type, 1, 4) + 40;
  center_zombie(g, ex, ey);

  prepare_plant();
  REQUIRE(w->plant(plant_type::cherry_bomb, 1, 4));
  wait_for_explosion();

  CHECK_FALSE(is_zombie_dead(g));
  CHECK(g.hp == 3000 - 1800);

  prepare_plant();
  REQUIRE(w->plant(plant_type::cherry_bomb, 1, 4));
  wait_for_explosion();

  CHECK(is_zombie_dead(g));
}

TEST_CASE_METHOD(CherryBombTest, "CherryBombGigaGargantuarKillCount",
                 "[CherryBomb]") {
  w->reset();
  w->scene.stop_spawn = true;
  zombie &gg = w->zombie_factory.create(zombie_type::giga_gargantuar, 1);

  int ex = get_explosion_x(4);
  int ey = get_y_by_row_and_col(w->scene.type, 1, 4) + 40;
  center_zombie(gg, ex, ey);

  w->select_plants({plant_type::cherry_bomb});

  for (int i = 0; i < 3; ++i) {
    prepare_plant();
    REQUIRE(w->plant(plant_type::cherry_bomb, 1, 4));
    wait_for_explosion();
    CHECK_FALSE(is_zombie_dead(gg));
  }

  prepare_plant();
  REQUIRE(w->plant(plant_type::cherry_bomb, 1, 4));
  wait_for_explosion();

  CHECK(is_zombie_dead(gg));
}

TEST_CASE_METHOD(CherryBombTest, "CherryBombOneShotOtherZombies",
                 "[CherryBomb]") {
  std::vector<zombie_type> ordinary_zombies = {
      zombie_type::zombie,        zombie_type::flag,
      zombie_type::conehead,      zombie_type::pole_vaulting,
      zombie_type::buckethead,    zombie_type::newspaper,
      zombie_type::screendoor,    zombie_type::football,
      zombie_type::dancing,       zombie_type::ducky_tube,
      zombie_type::snorkel,       zombie_type::zomboni,
      zombie_type::dolphin_rider, zombie_type::jack_in_the_box,
      zombie_type::balloon,       zombie_type::digger,
      zombie_type::pogo,          zombie_type::yeti,
      zombie_type::bungee,        zombie_type::ladder,
      zombie_type::catapult};

  for (auto type : ordinary_zombies) {
    w->reset();
    w->scene.stop_spawn = true;

    unsigned int row = 1;
    if (type == zombie_type::snorkel || type == zombie_type::dolphin_rider ||
        type == zombie_type::ducky_tube) {
      row = 2;
    }

    zombie &z = w->zombie_factory.create(type, row);
    z.row = row;

    int ex = get_explosion_x(4);
    int ey = get_y_by_row_and_col(w->scene.type, row, 4) + 40;

    if (type == zombie_type::bungee) {
      z.status = zombie_status::bungee_grab;
      z.bungee_col = 4;
      z.dy = 0;
      z.hit_box.offset_x = 0;
      z.hit_box.offset_y = 0;
      z.reanim.fps = 0;
    } else if (type == zombie_type::balloon) {
      z.status = zombie_status::balloon_flying;
      z.reanim.fps = 0;
    } else if (type == zombie_type::yeti) {
      z.status = zombie_status::walking;
      z.countdown.action = 1000;
      z.reanim.fps = 0;
    }

    center_zombie(z, ex, ey);

    prepare_plant();
    if (row == 2 || row == 3) {
      REQUIRE(w->plant(plant_type::lily_pad, row, 4));
    }
    REQUIRE(w->plant(plant_type::cherry_bomb, row, 4));
    wait_for_explosion();

    INFO("Testing zombie type: " << zombie::type_to_string(type)
                                 << " Status: " << static_cast<int>(z.status)
                                 << " Row: " << z.row << " HP: " << z.hp);
    CHECK(is_zombie_dead(z));
  }
}
