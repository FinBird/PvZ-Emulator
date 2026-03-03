#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/projectile.h"
#include "object/zombie.h"
#include "world.h"
#include <memory>

using namespace pvz_emulator;
using namespace pvz_emulator::object;

class PeaFamilyTest {
protected:
  std::unique_ptr<world> w;

  PeaFamilyTest() {
    w = std::make_unique<world>(scene_type::day);
    w->reset();
    w->scene.stop_spawn = true;
    // 消除随机性
    w->scene.disable_plant_boot_delay = true; // 1. 禁用植物启动随机延迟
    w->scene.lock_dx = true;                  // 2. 锁定僵尸移动速度
    w->scene.lock_dx_val = 0.28F;             // 3. 设定固定速度 (原版平均值)
  }

  int count_projectiles(projectile_type type) {
    int count = 0;
    for (auto &proj : w->scene.projectiles) {
      if (!proj.is_disappeared && proj.type == type)
        count++;
    }
    return count;
  }
};

TEST_CASE_METHOD(PeaFamilyTest, "SnowPeaDeathCoordinateContrast",
                 "[PeaFamilyTest]") {
  w->plant_factory.create(plant_type::pea_shooter, 1, 0);
  auto &z1 = w->zombie_factory.create(zombie_type::zombie, 1);
  z1.x = 700.0F;
  z1.int_x = 700;

  w->plant_factory.create(plant_type::snow_pea, 3, 0);
  auto &z2 = w->zombie_factory.create(zombie_type::zombie, 3);
  z2.x = 700.0F;
  z2.int_x = 700;

  int safety_timeout = 10000;
  while ((!z1.is_dead || !z2.is_dead) && safety_timeout-- > 0)
    w->update();

  INFO("Slowed zombie should die significantly farther from the house");
  CHECK(z2.x > z1.x + 100.0F);
  CHECK(z1.is_dead);
  CHECK(z2.is_dead);
}