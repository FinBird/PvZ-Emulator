#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <chrono>
#include <exception>
#include <format>
#include <iostream>

#include <windows.h>
#endif
#include "object/plant.h"
#include "object/zombie.h"
#include "system/util.h"
#include "world.h"
using namespace pvz_emulator;
using namespace pvz_emulator::object;

void run_simulation(bool enable_print) {
  world w(scene_type::day);
  w.scene.stop_spawn = true;
  auto &p = w.plant_factory.create(plant_type::pea_shooter, 2, 1);
  p.countdown.generate = 0;

  auto &z = w.zombie_factory.create(zombie_type::zombie, 2);
  z.x = 700.0F;
  z.y = system::zombie_init_y(w.scene.type, z, z.row);
  int z_id = w.scene.zombies.get_index(z);

  if (enable_print) {
    std::cout << "=== 战斗开始 ===\n";
    std::cout << std::format("植物初始 HP: {}, 僵尸初始 HP: {}\n", p.hp, z.hp);
  }

  for (int frame = 1; frame <= 2000; ++frame) {
    w.update();

    if (enable_print) {
      if (frame % 100 == 0) {
        auto *z_ptr = w.scene.zombies.get(z_id);
        if ((z_ptr != nullptr) && !z_ptr->is_dead) {
          int col = system::get_col_by_x(static_cast<int>(z_ptr->x));
          std::cout << std::format(
              "[Frame {:>4}] 僵尸位置: {:6.2f} | 所在列: {} | 僵尸 HP: {:3}\n",
              frame, z_ptr->x, col, z_ptr->hp);
        }
      }
    }
  }

  if (enable_print) {
    std::cout << "=== 模拟结束 ===\n";
  }
}

void init_console() {
#ifdef _WIN32
  SetConsoleOutputCP(65001);
#endif
}

int main() {
  init_console();
  try {
    // 1. 第一次运行（带输出）
    run_simulation(true);

    // 2. 预热
    for (int i = 0; i < 10; ++i)
      run_simulation(false);

    // 3. 性能测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
      run_simulation(false);
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> diff = end - start;
    std::cout << std::format("{:.6f}\n", diff.count());

  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
  return 0;
}