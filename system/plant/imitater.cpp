#include "plant.h"
#include "system/plant/plant_factory.h"

namespace pvz_emulator::system {

using namespace pvz_emulator::object;

void plant_imitater::update(plant& p) {
    if (p.is_dead || p.is_smashed) {
        return;
    }
    if (p.status == plant_status::imitater_morphing) {
        p.edible = plant_edible_status::invisible_and_not_edible;

        if (p.reanim.n_repeated > 0) {
            plant_factory plant_factory(scene);
            plant_type target_type = p.imitater_target;
            unsigned int r = p.row;
            unsigned int c = p.col;
            plant_factory.destroy(p);

            auto &new_plant= plant_factory.create(target_type, r, c);

            new_plant.is_imitater = true;
        }
    } else {
        if (p.countdown.status > 0) {
            p.countdown.status--;
            p.edible = plant_edible_status::visible_and_edible;
        }
        if (p.countdown.status == 0) {
            p.status = plant_status::imitater_morphing;
            p.set_reanim(plant_reanim_name::anim_explode, reanim_type::once, 26);
            p.edible = plant_edible_status::invisible_and_not_edible;
        }
    }
}

void plant_imitater::init(
    plant & p,
    plant_type target,
    unsigned int row,
    unsigned int col)
{
    plant_base::init(p, plant_type::imitater, row, col, target);
    p.reanim.fps = rng.randfloat(25, 30);
    p.countdown.effect = 200;
    p.status = plant_status::idle;
    p.edible = plant_edible_status::visible_and_edible;
}

}