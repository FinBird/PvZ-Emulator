#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/zombie.h"
#include "system/util.h"
#include "world.h"

using namespace pvz_emulator::object;

class JalapenoTest {
protected:
  JalapenoTest() {
    w = std::make_unique<pvz_emulator::world>(
        scene_type::day); // 使用 day 场景，使得 0~4 行全合法，避免被强制分流
    w->reset();
    w->scene.stop_spawn = true;
  }
  std::unique_ptr<pvz_emulator::world> w;
  plant *create_plant(plant_type type, unsigned int row, unsigned int col) {
    return &w->plant_factory.create(type, row, col);
  }
};

TEST_CASE_METHOD(JalapenoTest,
                 "Jalapeno burns row instantly but leaves Gargantuars alive",
                 "[Jalapeno]") {
  w->scene.sun.sun = 9999;

  // 在第 2 行（目标行）生成各类僵尸，强制覆盖行数并同步实际 y 坐标
  zombie &z_normal = w->zombie_factory.create(zombie_type::zombie, 2);
  z_normal.row = 2;
  z_normal.x = 200;
  z_normal.int_x = 200;
  z_normal.y = pvz_emulator::system::zombie_init_y(w->scene.type, z_normal, 2);

  zombie &z_cone = w->zombie_factory.create(zombie_type::conehead, 2);
  z_cone.row = 2;
  z_cone.x = 300;
  z_cone.int_x = 300;
  z_cone.y = pvz_emulator::system::zombie_init_y(w->scene.type, z_cone, 2);

  zombie &z_bucket = w->zombie_factory.create(zombie_type::buckethead, 2);
  z_bucket.row = 2;
  z_bucket.x = 400;
  z_bucket.int_x = 400;
  z_bucket.y = pvz_emulator::system::zombie_init_y(w->scene.type, z_bucket, 2);

  zombie &z_garg = w->zombie_factory.create(zombie_type::gargantuar, 2);
  z_garg.row = 2;
  z_garg.x = 500;
  z_garg.int_x = 500;
  z_garg.y = pvz_emulator::system::zombie_init_y(w->scene.type, z_garg, 2);
  unsigned int garg_initial_hp = z_garg.hp; // 3000

  zombie &z_giga = w->zombie_factory.create(zombie_type::giga_gargantuar, 2);
  z_giga.row = 2;
  z_giga.x = 600;
  z_giga.int_x = 600;
  z_giga.y = pvz_emulator::system::zombie_init_y(w->scene.type, z_giga, 2);
  unsigned int giga_initial_hp = z_giga.hp; // 6000

  // 在第 1 行和第 3 行生成参照僵尸，验证火爆辣椒是否会跨行判定
  zombie &z_other_row1 = w->zombie_factory.create(zombie_type::zombie, 1);
  z_other_row1.row = 1;
  z_other_row1.x = 300;
  z_other_row1.int_x = 300;
  z_other_row1.y =
      pvz_emulator::system::zombie_init_y(w->scene.type, z_other_row1, 1);

  zombie &z_other_row3 = w->zombie_factory.create(zombie_type::zombie, 3);
  z_other_row3.row = 3;
  z_other_row3.x = 300;
  z_other_row3.int_x = 300;
  z_other_row3.y =
      pvz_emulator::system::zombie_init_y(w->scene.type, z_other_row3, 3);

  // 在第 2 行种下火爆辣椒
  plant *jalapeno = create_plant(plant_type::jalapeno, 2, 4);

  // 运行游戏逻辑，等待火爆辣椒爆炸 (countdown.effect 为 100)
  int safe = 0;
  while (!jalapeno->is_dead && safe < 300) {
    w->update();
    safe++;
  }

  // 1. 验证辣椒本身已消耗殆尽
  CHECK(jalapeno->is_dead == true);

  // 2. 验证同行的普通/防具僵尸全部被秒杀
  CHECK(z_normal.is_dead == true);
  CHECK(z_cone.is_dead == true);
  CHECK(z_bucket.is_dead == true);

  // 3. 验证同行的巨人和红眼巨人未死，并且精确扣除 1800 灰烬伤害
  CHECK(z_garg.is_dead == false);
  CHECK(z_garg.hp == garg_initial_hp - 1800);

  CHECK(z_giga.is_dead == false);
  CHECK(z_giga.hp == giga_initial_hp - 1800);

  // 4. 验证其他行的僵尸不受影响（安全无恙）
  CHECK(z_other_row1.is_dead == false);
  CHECK(z_other_row3.is_dead == false);
}