#pragma once
#include "common.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace pvz_emulator::object {

enum class projectile_type : std::int32_t {
    none = -1,
    pea = 0x0,         // 豌豆
    snow_pea = 0x1,    // 冰豌豆
    cabbage = 0x2,     // 卷心菜
    melon = 0x3,       // 西瓜
    puff = 0x4,        // 孢子（小喷菇）
    wintermelon = 0x5, // 冰西瓜
    fire_pea = 0x6,    // 火豌豆
    star = 0x7,        // 星星
    cactus = 0x8,      // 尖刺
    basketball = 0x9,  // 篮球
    kernel = 0xA,      // 玉米粒
    cob_cannon = 0xB,  // 玉米炮弹
    butter = 0xC,      // 黄油
    zombie_pea = 0xD   // 僵尸豌豆 (TODO)
};

enum class projectile_motion_type : std::uint8_t {
    straight = 0,      // 水平向右直线（豌豆）
    parabola = 1,      // 抛物线（投手）
    switch_way = 2,    // 三线偏转（三线射手）
    bee = 3,           // 蜜蜂子弹？
    bee_backwards = 4, // 反向蜜蜂子弹？
    puff = 5,          // 水平向右（孢子，一段时间后消失）
    left_straight = 6, // 水平向左
    starfruit = 7,     // 斜向运动
    homing = 9        // 追踪轨道（香蒲）
};

class scene;
class zombie;

class projectile {
public:
    int from_plant;
    projectile_type type;
    projectile_motion_type motion_type;

    int int_x;
    int int_y;

    int row;

    float y;
    float x;
    float shadow_y;

    float dx;

    float dy1;
    float dy2;

    float ddy;
    float dddy;

    struct {
        int width;
        int height;
    } attack_box;

    unsigned int flags;

    unsigned int time_since_created;
    unsigned int countdown;

    int last_torchwood_col;

    float cannon_x;
    int cannon_row;

    int target;

    bool is_visible;
    bool is_disappeared;

    const static unsigned int DAMAGE[14];

    bool is_freeable() {
        return is_disappeared;
    }

    void get_attack_box(rect& r) const;

    unsigned int get_flags_with_zombie(const zombie& z) const;

    void to_json(
        object::scene& scene,
        rapidjson::Writer<rapidjson::StringBuffer>& writer);

    static const char* type_to_string(projectile_type type);
    static const char* motion_type_to_string(projectile_motion_type type);
};

}
