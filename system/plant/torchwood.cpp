#include "damage.h"
#include "plant.h"

namespace pvz_emulator::system {

using namespace pvz_emulator::object;

void plant_torchwood::update(plant & p) {
    rect pr;
    p.get_attack_box(pr);

    for (auto& proj : scene.projectiles) {
        if (proj.type != projectile_type::pea &&
            proj.type != projectile_type::snow_pea ||
            proj.type == projectile_type::fire_pea ||
            proj.row != p.row)
        {
            continue;
        }

        rect pjr;
        pjr.width = 28;
        pjr.height = 28;
        pjr.x = static_cast<int>(proj.x) - 14;
        pjr.y = static_cast<int>(proj.y) - 14;

        if (pr.intersects(pjr)) {
            float overlap = pr.get_overlap_len(pjr);
            if (overlap >= 10.0F && proj.last_torchwood_col != static_cast<int>(p.col)) {
                if (proj.type == projectile_type::pea) {
                    proj.type = projectile_type::fire_pea;
                } else if (proj.type == projectile_type::snow_pea) {
                    proj.type = projectile_type::pea;
                    proj.flags &= ~(static_cast<unsigned int>(zombie_damage_flags::freeze));
                }
                proj.last_torchwood_col = static_cast<int>(p.col);
            }
        }
    }
}

}