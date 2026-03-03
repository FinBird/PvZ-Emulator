#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/zombie.h"
#include "world.h"
#include <memory>

using namespace pvz_emulator::object;

class ChomperTest {
protected:
  ChomperTest() {
    w = std::make_unique<pvz_emulator::world>(scene_type::day);
    w->reset();
    w->scene.stop_spawn = true;
    w->scene.sun.sun = 9999;
  }

  std::unique_ptr<pvz_emulator::world> w;

  void run_until_status_changes(plant *p, plant_status target_status,
                                int max_steps = 1000) {
    int steps = 0;
    while (p->status != target_status && steps < max_steps) {
      w->update();
      steps++;
    }
  }
};

// 1. 基础逻辑测试：正常僵尸被吞噬并进入咀嚼
TEST_CASE_METHOD(ChomperTest, "BasicEatAndChew", "[ChomperTest]") {
  auto &chomper = w->plant_factory.create(plant_type::chomper, 2, 1);
  zombie &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = chomper.x + 60.0F;
  z.int_x = (int)z.x;

  run_until_status_changes(&chomper, plant_status::chomper_bite_success);
  CHECK(z.is_dead);

  // 关键：模拟动画播完，系统会自动转入 chew 状态
  chomper.reanim.n_repeated = 1;
  w->update();

  CHECK(chomper.status == plant_status::chomper_chew);
  CHECK(chomper.countdown.status == 4000);
}

// 2. 巨人致死计数：3000HP / 40伤害 = 75次。
TEST_CASE_METHOD(ChomperTest, "GargantuarCumulativeDamageTest",
                 "[ChomperTest]") {
  auto &chomper = w->plant_factory.create(plant_type::chomper, 2, 1);
  zombie &g = w->zombie_factory.create(zombie_type::gargantuar, 2);
  g.x = chomper.x + 85.0F;
  g.int_x = (int)g.x;
  int bite_count = 0;

  for (int i = 0; i < 15000; ++i) {
    g.x = chomper.x + 85.0F;
    g.int_x = (int)g.x;
    plant_status prev_status = chomper.status;
    w->update();

    if (prev_status == plant_status::chomper_bite_begin &&
        chomper.status == plant_status::chomper_bite_fail) {
      bite_count++;
      // 关键：动画播放完成回到 wait 状态，准备下一口
      chomper.reanim.n_repeated = 1;
      if (bite_count == 38) {
        CHECK(g.hp < 1500);
      }
    }
    if (g.is_dead)
      break;
  }

  CHECK(bite_count >= 75);
  CHECK(bite_count <= 76);
  CHECK(g.is_dead);
}

// 3. 机械单位秒杀逻辑：冰车、投石车
TEST_CASE_METHOD(ChomperTest, "MechanicalUnitsInstakill", "[ChomperTest]") {
  zombie_type mechanics[] = {zombie_type::zomboni, zombie_type::catapult};

  for (auto z_type : mechanics) {
    w->reset();
    auto &chomper = w->plant_factory.create(plant_type::chomper, 2, 2);
    zombie &z = w->zombie_factory.create(z_type, 2);
    z.x = chomper.x + 85.0F;
    z.int_x = (int)z.x;

    run_until_status_changes(&chomper, plant_status::chomper_bite_success);
    INFO("Vehicle should be eaten!");
    CHECK(z.is_dead);
  }
}

// 4. 无法秒杀测试：气球、跳跳
TEST_CASE_METHOD(ChomperTest, "UnbiteableUnits", "[ChomperTest]") {
  // 气球测试
  auto &c1 = w->plant_factory.create(plant_type::chomper, 2, 3);
  zombie &b = w->zombie_factory.create(zombie_type::balloon, 2);
  b.status = zombie_status::balloon_flying;
  b.x = c1.x + 40.0F;
  b.int_x = (int)b.x;

  for (int i = 0; i < 200; ++i)
    w->update();
  CHECK_FALSE(b.is_dead);
  CHECK(c1.status != plant_status::chomper_chew);

  // 跳跳测试
  auto &c2 = w->plant_factory.create(plant_type::chomper, 1, 3);
  zombie &p = w->zombie_factory.create(zombie_type::pogo, 1);
  p.status = zombie_status::pogo_with_stick;
  p.x = c2.x + 40.0F;
  p.int_x = (int)p.x;

  for (int i = 0; i < 200; ++i)
    w->update();
  CHECK_FALSE(p.is_dead);
  CHECK(c2.status != plant_status::chomper_chew);
}

// 5. 魅惑单位测试：大嘴花应忽略魅惑僵尸
TEST_CASE_METHOD(ChomperTest, "IgnoreHypnotized", "[ChomperTest]") {
  auto &chomper = w->plant_factory.create(plant_type::chomper, 2, 1);
  zombie &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.is_hypno = true;
  z.x = chomper.x + 40.0F;
  z.int_x = (int)z.x;

  for (int i = 0; i < 100; ++i)
    w->update();

  CHECK(chomper.status == plant_status::wait);
  CHECK_FALSE(z.is_dead);
}

// 6. 潜水僵尸测试：水下无视，露头可吃
TEST_CASE_METHOD(ChomperTest, "SnorkelInteraction", "[ChomperTest]") {
  auto pool_w = std::make_unique<pvz_emulator::world>(scene_type::pool);
  pool_w->reset();
  pool_w->scene.stop_spawn = true;
  pool_w->plant_factory.create(plant_type::lily_pad, 2, 5);
  auto &chomper = pool_w->plant_factory.create(plant_type::chomper, 2, 5);

  zombie &z = pool_w->zombie_factory.create(zombie_type::snorkel, 2);
  z.is_in_water = true;
  z.status = zombie_status::snorkel_swim;
  z.x = chomper.x + 85.0F;
  z.int_x = (int)z.x;

  // 潜水中，不应触发攻击
  for (int i = 0; i < 50; ++i)
    pool_w->update();
  CHECK(chomper.status == plant_status::chomper_bite_begin);

  // 暴露后应被吃
  z.is_eating = true;
  for (int i = 0; i < 200; ++i) {
    if (chomper.status == plant_status::chomper_bite_begin)
      break;
    pool_w->update();
  }
  CHECK(chomper.status == plant_status::chomper_bite_begin);
}

// 7. 反向矿工测试：无视朝向吃掉
TEST_CASE_METHOD(ChomperTest, "ReverseDiggerInstakill", "[ChomperTest]") {
  auto &chomper = w->plant_factory.create(plant_type::chomper, 2, 1);
  zombie &z = w->zombie_factory.create(zombie_type::digger, 2);
  z.status = zombie_status::digger_walk_right;
  z.x = chomper.x + 40.0F;
  z.int_x = (int)z.x;
  z.has_item_or_walk_left = true;

  run_until_status_changes(&chomper, plant_status::chomper_bite_success);
  CHECK(z.is_dead);
}

// 8. 验证大嘴花盲区逻辑
TEST_CASE_METHOD(ChomperTest, "DiggerEmergenceBlindSpot", "[ChomperTest]") {
  auto &chomper = w->plant_factory.create(plant_type::chomper, 2, 1);
  // 矿工在 0 列出土 (x=20)
  zombie &z = w->zombie_factory.create(zombie_type::digger, 2);
  z.status = zombie_status::digger_landing;
  z.x = 20.0F;
  z.int_x = 20;

  for (int i = 0; i < 200; ++i) {
    w->update();
    INFO("Chomper bit a zombie that was too far behind!");
    REQUIRE(chomper.status != plant_status::chomper_bite_begin);
  }
}

// 9. 验证大嘴花成功拦截
TEST_CASE_METHOD(ChomperTest, "DiggerWalksIntoChomperRange", "[ChomperTest]") {
  auto &chomper = w->plant_factory.create(plant_type::chomper, 2, 1);
  zombie &z = w->zombie_factory.create(zombie_type::digger, 2);
  z.status = zombie_status::digger_walk_right;
  z.x = 190.0F;
  z.int_x = 190;
  z.has_item_or_walk_left = true;

  int safety = 50;
  while (chomper.status == plant_status::wait && ((safety--) != 0))
    w->update();
  CHECK(chomper.status == plant_status::chomper_bite_begin);

  for (int i = 0; i < 100; ++i)
    w->update();
  CHECK(z.is_dead);
}