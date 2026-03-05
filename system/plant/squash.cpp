#include <cassert>
#include "plant.h"
#include "system/util.h"
#include "system/damage.h"
#include "system/plant/plant_factory.h"
#include "system/zombie/zombie.h"

namespace pvz_emulator::system {

using namespace pvz_emulator::object;

void plant_squash::kill(plant& p) {
    auto flags = p.get_attack_flags();
    rect pr;
    p.get_attack_box(pr);
    damage damage(scene);

    for (auto& z : scene.zombies) {
        if (!damage.can_be_attacked(z, flags) || z.row != p.row) {
            continue;
        }

        rect zr;
        z.get_hit_box(zr);
        auto d = pr.get_overlap_len(zr);

        if (d > static_cast<float>(z.type == zombie_type::football ? -20 : 0)) {
            damage.take(z, 1800,
                zombie_damage_flags::disable_ballon_pop |
                zombie_damage_flags::not_reduce);
        }
    }
}

zombie* plant_squash::find_target(plant& p) {
    rect pr;
    p.get_attack_box(pr);

    int min_range = 9999;
    zombie* target = nullptr;

    for (auto &z : scene.zombies) {
        if (z.row == p.row &&
            z.is_not_dying &&
            !is_target_of_kelp(scene, z) &&
            damage(scene).can_be_attacked(z, p.get_attack_flags(false)))
        {
            if (z.status == zombie_status::pole_valuting_jumping ||
                z.status == zombie_status::snorkel_jump_in_the_pool ||
                z.status == zombie_status::dolphin_jump_in_pool ||
                z.status == zombie_status::dolphin_ride ||
                z.status == zombie_status::dolphin_jump)
            {
                continue;
            }

            rect zr;
            z.get_hit_box(zr);

            if (z.status == zombie_status::pole_valuting_running &&
                zr.x >= p.x + 20)
            {
                continue;
            }

            int overlap_range = -pr.get_overlap_len(zr);
            int max_range = z.is_eating ? 110 : 70;

            if (overlap_range > max_range) {
                continue;
            }

            int b = pr.x;

            if (z.status == zombie_status::pole_vaulting_walking ||
                z.status == zombie_status::pole_valuting_running ||
                z.status == zombie_status::dolphin_walk_in_pool ||
                z.type == zombie_type::imp ||
                z.type == zombie_type::football)
            {
                b -= 60;
            }

            if (z.is_walk_right() || zr.width + zr.x >= b) {
                if (p.target == scene.zombies.get_index(z)) {
                    return &z;
                }

                if (target == nullptr || overlap_range < min_range) {
                    min_range = overlap_range;
                    target = &z;
                }
            }
        }
    }

    return target;
}

int plant_squash::get_jump_up_pos(plant& p, int a, int b) {
    float r = (50 - p.countdown.status) / 30.0F;

    if (r <= 0.0F) {
        return a;
    } else if (r > 1.0F) {
        return b;
    } else {
        r = (3.0F * r * r) - (2.0F * r * r * r);
        return static_cast<int>(round(((b - a) * r) + a));
    }
}

void plant_squash::update(plant& p) {
    int y = get_y_by_row_and_col(
        scene.type,
        p.row,
        std::max(0, get_col_by_x(p.cannon.x))) + 8;

    switch (p.status) {
    case plant_status::squash_look:
        if (p.countdown.status == 0) {
            p.set_reanim(plant_reanim_name::anim_jumpup, reanim_type::once, 24);
            p.status = plant_status::squash_jump_up;
            p.countdown.status = 30;
        }
        break;

    case plant_status::squash_jump_up:
        if (p.countdown.status == 0) {
            if (auto *z = find_target(p)) {
                p.cannon.x = static_cast<int>(
                    zombie_base(scene).predict_after(*z, 30) -
                    (p.attack_box.width / 2.0F));
            }
            p.status = plant_status::squash_stop_in_the_air;
            p.countdown.status = 50;

            if (scene.plant_map[p.row][p.col].content == &p) {
                scene.plant_map[p.row][p.col].content = nullptr;
            }
        }
        break;

    case plant_status::squash_stop_in_the_air:
        p.x = get_jump_up_pos(p, get_x_by_col(p.col), p.cannon.x);
        p.y = get_jump_up_pos(
            p,
            get_y_by_row_and_col(scene.type, p.row, p.col),
            y - 120);

        if (p.countdown.status == 0) {
            p.set_reanim(plant_reanim_name::anim_jumpdown, reanim_type::once, 60);
            p.status = plant_status::squash_jump_down;
            p.countdown.status = 10;

            if (scene.plant_map[p.row][p.col].content == &p) {
                scene.plant_map[p.row][p.col].content = nullptr;
            }
        }
        break;


    case plant_status::squash_jump_down:{
        float r = (10.0F - p.countdown.status) / 10.0F;
        r = std::max(0.0F, std::min(1.0F, r));
        r = (3.0F * r * r) - (2.0F * r * r * r);
        
        p.y = static_cast<int>(round(120.0F * r)) + y - 120;

        if (p.countdown.status == 5) {
            kill(p);
        } else if (p.countdown.status == 0) {
            if (scene.is_water_grid(p.row, p.col)) {
                plant_factory(scene).destroy(p);
            } else {
                p.status = plant_status::squash_crushed;
                p.countdown.status = 100;
                if (scene.plant_map[p.row][p.col].content == &p) {
                    scene.plant_map[p.row][p.col].content = nullptr;
                }
            }
        }
        break;
    }

    case plant_status::squash_crushed:
        if (p.countdown.status == 0) {
            plant_factory(scene).destroy(p);
        }
        break;

    case plant_status::idle:
        if (auto *z = find_target(p)) {
            p.target = scene.zombies.get_index(*z);
            rect zr;
            z->get_hit_box(zr);

            p.cannon.x = static_cast<int>(zr.x + (zr.width / 2.0F) - (p.attack_box.width / 2.0F));
            p.status = plant_status::squash_look;
            p.countdown.status = 80;

            if (p.cannon.x >= p.x) {
                p.set_reanim(plant_reanim_name::anim_lookright, reanim_type::once, 24);
            } else {
                p.set_reanim(plant_reanim_name::anim_lookleft, reanim_type::once, 24);
            }
        }
        break;

    default:
        assert(false);
    }
}

}