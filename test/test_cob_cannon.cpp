#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/zombie.h"
#include "system/plant/plant.h"
#include "system/plant/plant_system.h"
#include "system/util.h"
#include "world.h"
#include <chrono>
#include <iostream>

using namespace pvz_emulator;
using namespace pvz_emulator::object;

class CobCannonTest {
protected:
  CobCannonTest() {
    w = std::make_unique<world>(scene_type::day);
    w->reset();
    w->scene.stop_spawn = true;
    w->scene.sun.sun = 9999;
  }

  std::unique_ptr<world> w;

  // 辅助函数：在指定位置设置僵尸并同步坐标
  zombie &setup_zombie_at(unsigned int row, float x) {
    auto &z = w->zombie_factory.create(zombie_type::zombie, row);
    z.row = row;
    z.x = x;
    z.y = system::zombie_init_y(w->scene.type, z, row);
    z.int_x = static_cast<int>(z.x);
    z.int_y = static_cast<int>(z.y);
    return z;
  }

  plant *find_plant(plant_type type) {
    for (auto &p : w->scene.plants) {
      if (!p.is_dead && p.type == type)
        return &p;
    }
    return nullptr;
  }
};

// 1. 基础状态与充能测试
TEST_CASE_METHOD(CobCannonTest, "InitialStateAndCharge", "[CobCannonTest]") {
  w->select_plants({plant_type::kernelpult, plant_type::cob_cannon});
  w->scene.sun.sun = 2000;
  w->plant(plant_type::kernelpult, 2, 2);
  w->scene.cards[0].cold_down = 0;
  w->plant(plant_type::kernelpult, 2, 3);
  plant *cob = find_plant(plant_type::cob_cannon);
  if (cob == nullptr) {
      cob = w->plant_factory.plant(1, 2, 2);
  }

  REQUIRE(cob != nullptr);
  CHECK(cob->status == plant_status::cob_cannon_unarmed_idle);

  cob->countdown.status = 0;
  w->plant_system.update();
  CHECK(cob->status == plant_status::cob_cannon_charge);

  cob->reanim.n_repeated = 1;
  w->plant_system.update();
  CHECK(cob->status == plant_status::cob_cannon_armed_idle);
}

// 2. 发射逻辑测试
TEST_CASE_METHOD(CobCannonTest, "LaunchSequence", "[CobCannonTest]") {
  w->plant_factory.create(plant_type::kernelpult, 2, 2);
  w->plant_factory.create(plant_type::kernelpult, 2, 3);
  plant &cob = w->plant_factory.create(plant_type::cob_cannon, 2, 2);
  cob.status = plant_status::cob_cannon_armed_idle;

  system::plant_cob_cannon cob_logic(w->scene);
  bool ok = cob_logic.launch(cob, 600, 200);

  CHECK(ok);
  CHECK(cob.status == plant_status::cob_cannon_launch);
  CHECK(cob.countdown.launch == 206);

  // 第 204 次 update 后变为 2。
  // 第 205 次 update 后变为 1，逻辑触发发射产生炮弹。
  for (int i = 0; i < 204; ++i)
    w->update();
  CHECK(w->scene.projectiles.size() == 0);

  w->update(); // 第 205 帧：产生炮弹
  CHECK(w->scene.projectiles.size() == 1);
}

// 3. 定位限制测试
TEST_CASE_METHOD(CobCannonTest, "PlacementRestrictions", "[CobCannonTest]") {
  w->select_plants({plant_type::cob_cannon});
  CHECK_FALSE(w->plant_factory.can_plant(2, 2, plant_type::cob_cannon));

  w->plant_factory.create(plant_type::kernelpult, 2, 2);
  CHECK_FALSE(w->plant_factory.can_plant(2, 2, plant_type::cob_cannon));

  w->plant_factory.create(plant_type::kernelpult, 3, 2);
  CHECK_FALSE(w->plant_factory.can_plant(2, 2, plant_type::cob_cannon));

  w->plant_factory.create(plant_type::kernelpult, 2, 3);
  CHECK(w->plant_factory.can_plant(2, 2, plant_type::cob_cannon));
}

// 4. 同步销毁测试
TEST_CASE_METHOD(CobCannonTest, "SynchronizedDestruction", "[CobCannonTest]") {
  w->plant_factory.create(plant_type::kernelpult, 2, 2);
  w->plant_factory.create(plant_type::kernelpult, 2, 3);
  plant &cob = w->plant_factory.create(plant_type::cob_cannon, 2, 2);

  CHECK(w->scene.plant_map[2][2].content == &cob);
  CHECK(w->scene.plant_map[2][3].content == &cob);

  w->plant_factory.destroy(*w->scene.plant_map[2][3].content);

  CHECK(cob.is_dead);
  CHECK(w->scene.plant_map[2][2].content == nullptr);
  CHECK(w->scene.plant_map[2][3].content == nullptr);
}

// 5. 性能压力测试
TEST_CASE_METHOD(CobCannonTest, "ExplosionPerformanceStress",
                 "[CobCannonTest]") {
  const int ZOMBIE_COUNT = 500;
  for (int i = 0; i < ZOMBIE_COUNT; ++i) {
    setup_zombie_at(2, 700.0F + (i * 0.1F));
  }

  for (int i = 0; i < 4; ++i) {
    auto &proj =
        w->projectile_factory.create(projectile_type::cob_cannon, 2, 700, 200);
    proj.motion_type = projectile_motion_type::parabola;
    proj.flags = static_cast<unsigned int>(attack_flags::ground);
    proj.time_since_created = 25;
    proj.ddy = 1.0f;
    proj.dy1 = 0.0f;
  }

  auto start = std::chrono::high_resolution_clock::now();
  // 触发所有炮弹的碰撞判定
  w->projectile.update();
  auto end = std::chrono::high_resolution_clock::now();

  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();
  std::cout << "[Performance] 4 Cob Cannons hitting " << ZOMBIE_COUNT
            << " zombies took: " << duration << " us\n";

  // 验证结果：500个僵尸应该全部死亡
  int alive_count = 0;
  for (auto &z : w->scene.zombies) {
    if (!z.is_dead)
      alive_count++;
  }

  CHECK(alive_count == 0);
  // 性能阈值检查
  CHECK(duration < 2000);
}