#pragma once
#include "object/scene.h"
#include "rng.h"

namespace pvz_emulator::system {

class fog {
  object::scene &scene;
  system::rng rng;

public:
  fog(object::scene &s) : scene(s), rng(s) {}

  // 每帧更新雾状态
  void update();

  // 三叶草吹散所有雾
  void blow_away();

  // 判断某列是否被雾遮挡
  bool is_fogged(unsigned int col) const;

  // 路灯花/火炬树桩清除周围雾气
  void clear_fog_around_plants();
};

} // namespace pvz_emulator::system
