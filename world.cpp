#include "world.h"
#include <unordered_set>
#include <unordered_map>

#ifdef PVZEMU_BUILD_DEBUGGER
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#endif // PVZEMU_BUILD_DEBUGGER

#include "world.h"

using namespace pvz_emulator::object;

namespace pvz_emulator {

void world::clean_obj_lists() {
    scene.plants.shrink_to_fit();
    scene.zombies.shrink_to_fit();
    scene.projectiles.shrink_to_fit();
    scene.griditems.shrink_to_fit();
}

bool world::update() {
    if (scene.is_game_over) {
        return true;
    }

    scene.zombie_dancing_clock += 1;

    clean_obj_lists();

    griditem.update();

    plant_system.update();

    if (zombie.update()) {
        scene.is_game_over = true;
        return true;
    }

    projectile.update();

    for (auto& card : scene.cards) {
        if (card.cold_down > 0) {
            --card.cold_down;
        }
    }

    sun.update();

    if (!scene.stop_spawn) {
        spawn.update();
    }

    ice_path.update();

    if (scene.spawn.countdown.pool > 0) {
        --scene.spawn.countdown.pool;
    }

    if (endgame.update()) {
        spawn.reset();
        return true;
    } else {
        return false;
    }
}

bool world::update(const std::tuple<int, int, int> &action) {
    int op = std::get<0>(action);
    int row = std::get<1>(action);
    int col = std::get<2>(action);

    if (row >= 0 && row < scene.rows && col >= 0 && col < 9) {
        if (op == -2) {
            if (scene.plant_map[row][col].pumpkin) {
                plant_factory.destroy(*scene.plant_map[row][col].pumpkin);
            }
        } else if (op == -1) {
            if (scene.plant_map[row][col].coffee_bean) {
                plant_factory.destroy(*scene.plant_map[row][col].coffee_bean);
            } else if (scene.plant_map[row][col].content) {
                plant_factory.destroy(*scene.plant_map[row][col].content);
            } else if (scene.plant_map[row][col].base) {
                plant_factory.destroy(*scene.plant_map[row][col].base);
            }
        } else {
            for (int i = 0; i < 10; i++) {
                if (static_cast<int>(scene.cards[i].type) == op) {
                    plant(i, row, col);
                    break;
                }
            }
        }
    }

    return update();
}

void world::get_available_actions(
    const action_vector& actions,
    std::vector<int>& action_masks) const
{
    std::array<bool, static_cast<int>(plant_type::imitater) + 1> card_flags = {false};
    std::array<int, static_cast<int>(plant_type::imitater) + 1> card_index = {0};
    for (int i = 0; i < 10; i++) {
        if (scene.cards[i].type != plant_type::none && scene.cards[i].cold_down == 0) {
            card_flags[static_cast<int>(scene.cards[i].type)] = true;
            card_index[static_cast<int>(scene.cards[i].type)] = i;
        }
    }

    action_masks.resize(actions.size(), 0);

    for (int i = 0; i < actions.size(); i++) {
        int op = std::get<0>(actions[i]);
        int row = std::get<1>(actions[i]);
        int col = std::get<2>(actions[i]);

        if (row < 0 || row >= scene.rows || col < 0 || col >= 9) {
            continue;
        }

        if (op == -2) {
            if (scene.plant_map[row][col].pumpkin &&
                !scene.plant_map[row][col].pumpkin->is_dead &&
                !scene.plant_map[row][col].pumpkin->is_smashed)
            {
                action_masks[i] = 1;
            }
        } else if (op == -1) {
            if (scene.plant_map[row][col].coffee_bean &&
                !scene.plant_map[row][col].coffee_bean->is_dead &&
                !scene.plant_map[row][col].coffee_bean->is_smashed)
            {
                action_masks[i] = 1;
            } else if (scene.plant_map[row][col].content &&
               !scene.plant_map[row][col].content->is_dead &&
               !scene.plant_map[row][col].content->is_smashed)
            {
                action_masks[i] = 1;
            } else if (scene.plant_map[row][col].base &&
               !scene.plant_map[row][col].base->is_dead &&
               !scene.plant_map[row][col].base->is_smashed)
            {
                action_masks[i] = 1;
            }
        } else if (op >= 0 &&
            op <= static_cast<int>(plant_type::imitater) &&
            card_flags[op] &&
            plant_factory.can_plant(
                row,
                col,
                scene.cards[card_index[op]].type,
                scene.cards[card_index[op]].imitater_type))
        {
            action_masks[i] = 1;
        }
    }
}

void world::update_all(
    std::vector<world *>& w,
    const action_vector & actions,
    std::vector<int>& done,
    batch_action_masks& action_masks,
    unsigned int frames)
{
    std::atomic<decltype(w.size())> i = 0;
    done.resize(w.size());
    action_masks.resize(w.size());

    std::vector<std::thread> threads;
    for (unsigned int j = 0; j < std::thread::hardware_concurrency(); j++) {
        threads.emplace_back([&]() {
            for (auto k = i.fetch_add(1); k < w.size(); k = i.fetch_add(1)) {
                for (unsigned int l = 0; l < frames && !done[k]; l++) {
                    done[k] = w[k]->update();
                }
                w[k]->get_available_actions(actions, action_masks[k]);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
}

#ifdef PVZEMU_BUILD_DEBUGGER
void world::to_json(std::string& s) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sb);

    scene.to_json(writer);

    s = sb.GetString();
}
#endif // PVZEMU_BUILD_DEBUGGER

using namespace pvz_emulator::object;

bool world::select_plants(
    std::vector<plant_type> cards,
    plant_type imitater_type)
{
    auto n = cards.size();
    if (n > 10) {
        return false;
    }

    std::unordered_set<plant_type> unique_set;

    for (int i = 0; i < n; i++) {
        if (static_cast<int>(cards[i]) < 0 || static_cast<int>(cards[i]) > 48) {
            return false;
        }

        if (cards[i] == plant_type::imitater &&
            (static_cast<int>(imitater_type) < 0 ||
            static_cast<int>(imitater_type) > 47))
        {
            return false;
        }

        if (unique_set.find(cards[i]) != unique_set.end()) {
            return false;
        }

        unique_set.insert(cards[i]);
    }

    unsigned int prev_imitater_cd = 0;
    plant_type prev_imitater_type = plant_type::none;

    std::unordered_map<plant_type, unsigned int> cd;

    for (auto &card : scene.cards) {
        if (card.type != plant_type::none) {
            if (card.type == plant_type::imitater) {
                prev_imitater_type = card.imitater_type;
                prev_imitater_cd = card.cold_down;
            } else {
                cd[card.type] = card.cold_down;
            }
        }
    }

    for (int i = 0; i < 10; i++) {
        if (i < n) {
            if (cards[i] == plant_type::imitater) {
                scene.cards[i].type = plant_type::imitater;
                scene.cards[i].imitater_type = imitater_type;

                if (imitater_type == prev_imitater_type) {
                    scene.cards[i].cold_down = prev_imitater_cd;
                } else {
                    scene.cards[i].cold_down = 0;
                }
            } else {
                scene.cards[i].type = cards[i];
                scene.cards[i].imitater_type = plant_type::none;
                scene.cards[i].cold_down = cd.find(cards[i]) == cd.end() ?
                    0 :
                    cd[cards[i]];
            }
        } else {
            scene.cards[i].type =
                scene.cards[i].imitater_type =
                plant_type::none;

            scene.cards[i].cold_down = 0;
        }
    }

    return true;
}

bool world::plant(object::plant_type type, unsigned int row, unsigned int col) {
    for (int i = 0; i < 10; i++) {
        if (scene.cards[i].type == type) {
            return plant_factory.plant(i, row, col) != nullptr;
        }
    }

    return false;
}

}