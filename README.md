# PvZ-Emulator

Plants vs. Zombies Survival Endless Emulator.

A high-performance C++ emulator for *Plants vs. Zombies* Survival Endless mode.
This project emulates all plants and zombies (excluding Dr. Zomboss and the Zombie Bobsled Team, as they do not appear in Survival Endless). It supports all 6 main game scenes: Day, Night, Pool, Fog, Roof, and Moon Night.

This repository is a consolidated version that merges the history and features of two primary sources:

* [dnartz/PvZ-Emulator](https://github.com/dnartz/PvZ-Emulator)
* [Rottenham/pvz-emulator-examples](https://github.com/Rottenham/pvz-emulator-examples)

## Dependencies

The project relies on the following libraries:

* **[RapidJSON(24b5e7a)](https://github.com/Tencent/rapidjson)**: Used for high-speed state serialization.
* **[Catch2 (v3.13.0)](https://github.com/catchorg/Catch2)**: Used for the unit testing framework.

## Build and Run

This project utilizes [xmake](https://xmake.io) (v3.0.7) as the build system and can be compiled using the [Zig (v0.15.2)](https://ziglang.org/) toolchain for cross-platform stability.

To build and run the project, use the following commands:

* **Compile and run**:

  ```cmd
  xmake run
  ```

* **Run unit tests**:

  ```cmd
  xmake tests
  ```

## Usage

To use the emulator in your C++ project, include `pvz-emulator/world.h`. The simulation logic is thread-safe and optimized for high-frequency updates.

### Create and Update a Game Instance

```cpp
#include "world.h"

int main(void) {
    // Initialize a world with a specific scene
    pvz_emulator::world w(pvz_emulator::object::scene_type::pool);

    // Update game state (1 tick = 10ms)
    w.update();

    // The first wave of zombies will spawn after 6 seconds.
    for (int i = 0; i < 600; i++) {
        w.update();
    }

    return 0;
}
```

### Control Flags (Scene Configuration)

You can fine-tune the simulation behavior by modifying boolean flags within the `scene` object. These flags control game mechanics, bug emulation, and simulation constraints:

| Flag | Description |
| :--- | :--- |
| `is_zombie_dance` | Enables the "dance" movement logic for zombies. |
| `is_future_enabled` | Enables "future" mode effects (affecting certain zombie movement/mechanics). |
| `stop_spawn` | When `true`, the engine stops automatic wave generation. |
| `ignore_game_over` | If `true`, the simulation continues even if zombies enter the house. |
| `enable_split_pea_bug` | Emulates the original game bug where Split Pea fires incorrectly in certain frames. |
| `disable_garg_throw_imp` | Prevents Gargantuars from throwing Imps when their health is low. |
| `disable_crater` | Prevents Doom-shrooms from leaving craters on the grid. |
| `disable_plant_boot_delay` | Removes the random initial delay for plants (e.g., Peashooter fires immediately). |
| `lock_dx` | Forces zombies to move at a constant speed defined by `lock_dx_val`. |

## Example: Combat Simulation & Performance

The following example demonstrates a Peashooter vs. Zombie duel in the "Day" scene. It includes a performance benchmark calculating the execution time for 1,000 simulations.

### `example/main.cpp`

```cpp
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
    
    // Create a Peashooter at Row 2, Col 1
    auto &p = w.plant_factory.create(plant_type::pea_shooter, 2, 1);
    p.countdown.generate = 0; // Ready to shoot

    // Create a standard Zombie at Row 2
    auto &z = w.zombie_factory.create(zombie_type::zombie, 2);
    z.x = 700.0F;
    z.y = system::zombie_init_y(w.scene.type, z, z.row);
    int z_id = w.scene.zombies.get_index(z);

    if (enable_print) {
        std::cout << "=== Simulation Start ===\n";
        std::cout << std::format("Plant HP: {}, Zombie HP: {}\n", p.hp, z.hp);
    }

    for (int frame = 1; frame <= 2000; ++frame) {
        w.update();
        if (enable_print && frame % 100 == 0) {
            auto *z_ptr = w.scene.zombies.get(z_id);
            if ((z_ptr != nullptr) && !z_ptr->is_dead) {
                int col = system::get_col_by_x(static_cast<int>(z_ptr->x));
                std::cout << std::format(
                    "[Frame {:>4}] Pos: {:6.2f} | Col: {} | HP: {:3}\n",
                    frame, z_ptr->x, col, z_ptr->hp);
            }
        }
    }
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(65001); // Set UTF-8 console output
#endif

    try {
        // 1. Run simulation with output
        run_simulation(true);

        // 2. Performance Benchmark
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < 1000; ++i) {
            run_simulation(false);
        }
        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double> diff = end - start;
        std::cout << "\nTime for 1000 simulations: " << std::format("{:.6f}", diff.count()) << "s\n";

    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
```

```text
=== Simulation Start ===
Plant HP: 300, Zombie HP: 270
[Frame  100] Pos: 680.16 | Col: 8 | HP: 270
[Frame  200] Pos: 664.35 | Col: 7 | HP: 270
[Frame  300] Pos: 649.25 | Col: 7 | HP: 270
[Frame  400] Pos: 630.19 | Col: 7 | HP: 250
[Frame  500] Pos: 616.18 | Col: 7 | HP: 230
[Frame  600] Pos: 598.29 | Col: 6 | HP: 210
[Frame  700] Pos: 580.17 | Col: 6 | HP: 210
[Frame  800] Pos: 567.94 | Col: 6 | HP: 190
[Frame  900] Pos: 547.30 | Col: 6 | HP: 170
[Frame 1000] Pos: 530.39 | Col: 6 | HP: 150
[Frame 1100] Pos: 519.69 | Col: 5 | HP: 150
[Frame 1200] Pos: 496.30 | Col: 5 | HP: 130
[Frame 1300] Pos: 481.05 | Col: 5 | HP: 110
[Frame 1400] Pos: 471.47 | Col: 5 | HP:  90
[Frame 1500] Pos: 445.31 | Col: 5 | HP:  90
[Frame 1600] Pos: 431.66 | Col: 4 | HP:  70
[Frame 1700] Pos: 423.27 | Col: 4 | HP:  50
[Frame 1800] Pos: 395.48 | Col: 4 | HP:  50
[Frame 1900] Pos: 382.25 | Col: 4 | HP:  30
[Frame 2000] Pos: 372.15 | Col: 4 | HP:  10

Time for 1000 simulations: 0.248185s
```

## License

Please refer to the `LICENSE` file in the project folder for details.

## Contributing

Contributions are welcome!
If you find a bug or wish to emulate a missing game mechanic, please feel free to open an issue or submit a **Pull Request**.
