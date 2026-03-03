#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "world.h"

using namespace pvz_emulator;
using namespace pvz_emulator::object;

TEST_CASE("FogClearAndRestore", "[BloverFogTest]") {
  world w(scene_type::fog);
  w.reset();
  w.scene.stop_spawn = true;

  // 1. 验证初始状态有雾 (第5列及以后)
  CHECK(w.scene.fog.column[5] > 200);

  // 2. 种下三叶草
  w.plant_factory.create(plant_type::blover, 2, 2);

  // 3. 运行几帧直到 Blover 逻辑触发
  int ticks = 0;
  while (w.scene.fog.blown_countdown == 0 && ticks < 100) {
    w.update();
    ticks++;
  }

  CHECK(w.scene.fog.blown_countdown > 0);
  INFO("三叶草生效时迷雾应完全透明");
  CHECK(w.scene.fog.column[5] == 0);

  // 4. 等待倒计时结束，迷雾应恢复
  unsigned int wait_frames = w.scene.fog.blown_countdown + 10;
  for (unsigned int i = 0; i < wait_frames; ++i) {
    w.update();
  }

  CHECK(w.scene.fog.blown_countdown == 0);
  INFO("驱散效果结束后迷雾应恢复");
  CHECK(w.scene.fog.column[5] > 200);
}