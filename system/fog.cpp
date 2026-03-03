#include "fog.h"
#include <algorithm>

namespace pvz_emulator::system {

using namespace pvz_emulator::object;

void fog::update() {
  if (scene.type != scene_type::fog) {
    return;
  }

  auto &fog_data = scene.fog;

  // 1. 处理三叶草逻辑
  if (fog_data.blown_countdown > 0) {
    fog_data.blown_countdown--;
    fog_data.column.fill(0); // 只要计时器大于 0，所有列强制透明
    return;
  }

  // 2. 重置为默认雾浓状态 (右侧 5 列)
  // 只有在三叶草效果结束后，才需要根据植物位置动态计算
  for (int i = 0; i < 9; i++) {
    fog_data.column[i] = (i >= 4) ? 255 : 0;
  }

  // 3. 遍历植物，路灯花和火炬树桩实时更新可见区域
  for (auto &p : scene.plants) {
    // 死亡、被压扁、睡觉或隐形（如在空中或地下的特殊状态）的植物不驱雾
    if (p.is_dead || p.is_smashed || p.is_sleeping ||
        p.edible == plant_edible_status::invisible_and_not_edible) {
      continue;
    }

    int clear_range = 0;
    if (p.type == plant_type::plantern) {
      clear_range = 3; // 路灯花：左右3列
    } else if (p.type == plant_type::torchwood) {
      clear_range = 1; // 火炬树桩：左右1列
    }

    if (clear_range > 0) {
      int start = std::max(0, static_cast<int>(p.col) - clear_range);
      int end = std::min(8, static_cast<int>(p.col) + clear_range);
      for (int c = start; c <= end; c++) {
        fog_data.column[c] = 0; // 该区域设为透明
      }
    }
  }
}
void fog::blow_away() {
  if (scene.type != scene_type::fog)
    return;

  // 三叶草吹散时间约为 250 帧 (PC版) 到 500 帧之间
  // 这里采用 400 + 随机偏移
  scene.fog.blown_countdown = 400 + rng.randint(100);
  scene.fog.column.fill(0);
}

bool fog::is_fogged(unsigned int col) const {
  if (scene.type != scene_type::fog || col >= 9) {
    return false;
  }

  return scene.fog.column[col] > 128; // TODO:浓度超过一半视为被遮挡?
}

} // namespace pvz_emulator::system
