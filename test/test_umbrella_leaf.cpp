#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/zombie.h"
#include "system/util.h"
#include "world.h"

using namespace pvz_emulator::object;

class UmbrellaLeafTest {
protected:
  UmbrellaLeafTest() {
    w = std::make_unique<pvz_emulator::world>(
        scene_type::day); // 同样改为day场景防止意外
    w->reset();
    w->scene.stop_spawn = true;
  }
  std::unique_ptr<pvz_emulator::world> w;
  plant *create_plant(plant_type type, unsigned int row, unsigned int col) {
    return &w->plant_factory.create(type, row, col);
  }
};

TEST_CASE_METHOD(UmbrellaLeafTest,
                 "Umbrella Leaf intercepts Bungee and Basketball inside 3x3, "
                 "leaves outside vulnerable",
                 "[UmbrellaLeaf]") {
  // 核心防御点：在 (2, 4) 种下叶子保护伞
  // 其保护范围是 Row: 1~3, Col: 3~5
  plant *umbrella = create_plant(plant_type::umbrella_leaf, 2, 4);

  // 1. 设置保护范围内的目标
  plant *target_prot_bungee =
      create_plant(plant_type::sunflower, 1, 4); // (1,4) 在保护区
  plant *target_prot_basket =
      create_plant(plant_type::pea_shooter, 3, 3); // (3,3) 在保护区

  // 2. 设置保护范围外的目标
  plant *target_unprot_bungee =
      create_plant(plant_type::sunflower, 0, 4); // (0,4) Row 0 超出保护区
  plant *target_unprot_basket =
      create_plant(plant_type::pea_shooter, 4, 3); // (4,3) Row 4 超出保护区

  // 3. 生成攻击保护区内植物的僵尸，需覆盖初始化的随机选择坐标逻辑
  zombie &bungee_prot = w->zombie_factory.create(zombie_type::bungee, 1);
  bungee_prot.row = 1;
  bungee_prot.bungee_col = 4;
  bungee_prot.dy = 100;
  bungee_prot.x = (80 * bungee_prot.bungee_col) + 40;
  bungee_prot.y = pvz_emulator::system::zombie_init_y(
      w->scene.type, bungee_prot, bungee_prot.row);
  bungee_prot.int_x = (int)bungee_prot.x;
  bungee_prot.int_y = (int)bungee_prot.y;
  bungee_prot.status = zombie_status::bungee_target_drop;

  zombie &catapult_prot = w->zombie_factory.create(zombie_type::catapult, 3);
  catapult_prot.row = 3;
  catapult_prot.x = 650; // 前调坐标使得其快速进行抛射操作
  catapult_prot.y = pvz_emulator::system::zombie_init_y(
      w->scene.type, catapult_prot, catapult_prot.row);
  catapult_prot.int_x = 650;
  catapult_prot.int_y = (int)catapult_prot.y;
  catapult_prot.status = zombie_status::walking;

  // 4. 生成攻击保护区外植物的僵尸
  zombie &bungee_unprot = w->zombie_factory.create(zombie_type::bungee, 0);
  bungee_unprot.row = 0;
  bungee_unprot.bungee_col = 4;
  bungee_unprot.dy = 100;
  bungee_unprot.x = (80 * bungee_unprot.bungee_col) + 40;
  bungee_unprot.y = pvz_emulator::system::zombie_init_y(
      w->scene.type, bungee_unprot, bungee_unprot.row);
  bungee_unprot.int_x = (int)bungee_unprot.x;
  bungee_unprot.int_y = (int)bungee_unprot.y;
  bungee_unprot.status = zombie_status::bungee_target_drop;

  zombie &catapult_unprot = w->zombie_factory.create(zombie_type::catapult, 4);
  catapult_unprot.row = 4;
  catapult_unprot.x = 650;
  catapult_unprot.y = pvz_emulator::system::zombie_init_y(
      w->scene.type, catapult_unprot, catapult_unprot.row);
  catapult_unprot.int_x = 650;
  catapult_unprot.int_y = (int)catapult_unprot.y;
  catapult_unprot.status = zombie_status::walking;

  int safe = 0;
  bool umbrella_triggered = false;

  // 运行一段时间以覆盖蹦极掉落、抓取以及投石车投射等操作（上限约1500帧足够执行完整序列）
  while (safe < 1500) {
    w->update();

    // 如果蹦极抓取后的闲置等待态出现，我们需要快进一下（与原版测试代码类似优化执行速度）
    if (bungee_unprot.status == zombie_status::bungee_idle_after_drop &&
        bungee_unprot.countdown.action > 5) {
      bungee_unprot.countdown.action = 5;
    }
    if (bungee_prot.status == zombie_status::bungee_idle_after_drop &&
        bungee_prot.countdown.action > 5) {
      bungee_prot.countdown.action = 5;
    }

    // 检查叶子保护伞是否触发过张开动画（一旦拦截即会切换到 umbrella_leaf_block
    // 状态）
    if (umbrella->status == plant_status::umbrella_leaf_block) {
      umbrella_triggered = true;
    }

    safe++;
  }

  // == 验证检查 ==

  // 1. 保护伞应当成功触发了动画拦截，自身未受损
  CHECK(umbrella_triggered == true);
  CHECK(umbrella->hp == 300);

  // 2. 保护区内的植物应当完好无损（未被抓走且未受到投石伤害）
  CHECK(target_prot_bungee->edible == plant_edible_status::visible_and_edible);
  CHECK(target_prot_bungee->is_dead == false);
  CHECK(target_prot_basket->hp == 300); // 满血

  // 3. 蹦极僵尸（受保护行）应当被迫进入返回状态(raise) 或者
  // 处于目标掉落态且没抓到东西
  CHECK(bungee_prot.status == zombie_status::bungee_raise);

  // 4. 保护区外的植物应当受到应有的打击
  // (0,4)处防线外向日葵应当被没被防住的蹦极僵尸抓取（不可食用甚至已经被销毁）
  CHECK((target_unprot_bungee->edible ==
             plant_edible_status::invisible_and_not_edible ||
         target_unprot_bungee->is_dead == true));

  // (4,3)处防线外豌豆射手应当受到篮球的伤害（血量小于300基础值）
  CHECK(target_unprot_basket->hp < 300);
}