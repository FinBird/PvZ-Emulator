#pragma once
#include <array>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common.h"
#include "plant_reanim_data.h"

namespace pvz_emulator::object {

enum class plant_type {
    none = -1,
    pea_shooter = 0x0,     // 豌豆射手
    sunflower = 0x1,       // 向日葵
    cherry_bomb = 0x2,     // 樱桃炸弹
    wallnut = 0x3,         // 坚果墙
    potato_mine = 0x4,     // 土豆雷
    snow_pea = 0x5,        // 寒冰射手
    chomper = 0x6,         // 大嘴花
    repeater = 0x7,        // 双发射手
    puffshroom = 0x8,      // 小喷菇
    sunshroom = 0x9,       // 阳光菇
    fumeshroom = 0xA,      // 大喷菇
    grave_buster = 0xB,    // 墓碑吞噬者
    hypnoshroom = 0xC,     // 魅惑菇
    scaredyshroom = 0xD,   // 胆小菇
    iceshroom = 0xE,       // 寒冰菇
    doomshroom = 0xF,      // 毁灭菇
    lily_pad = 0x10,       // 睡莲
    squash = 0x11,         // 倭瓜
    threepeater = 0x12,    // 三线射手
    tangle_kelp = 0x13,    // 缠绕水草
    jalapeno = 0x14,       // 火爆辣椒
    spikeweed = 0x15,      // 地刺
    torchwood = 0x16,      // 火炬树桩
    tallnut = 0x17,        // 高坚果
    seashroom = 0x18,      // 海蘑菇
    plantern = 0x19,       // 路灯花
    cactus = 0x1a,         // 仙人掌
    blover = 0x1b,         // 三叶草
    split_pea = 0x1c,      // 裂荚射手
    starfruit = 0x1d,      // 杨桃
    pumpkin = 0x1e,        // 南瓜头
    magnetshroom = 0x1f,   // 磁力菇
    cabbagepult = 0x20,    // 卷心菜投手
    flower_pot = 0x21,     // 花盆
    kernelpult = 0x22,     // 玉米投手
    coffee_bean = 0x23,    // 咖啡豆
    garlic = 0x24,         // 大蒜
    umbrella_leaf = 0x25,  // 叶子保护伞
    marigold = 0x26,       // 金盏花
    melonpult = 0x27,      // 西瓜投手
    gatling_pea = 0x28,    // 机枪射手
    twin_sunflower = 0x29, // 双子向日葵
    gloomshroom = 0x2A,    // 忧郁菇
    cattail = 0x2B,        // 香蒲
    winter_melon = 0x2C,   // 冰瓜投手
    gold_magnet = 0x2D,    // 吸金磁
    spikerock = 0x2E,      // 地刺王
    cob_cannon = 0x2F,     // 玉米加农炮
    imitater = 0x30,       // 模仿者

    // IZombie 卡片类型
    seed_zombie = 0x3C,        // 普通僵尸
    seed_cone_head = 0x3D,     // 路障僵尸
    seed_pole_vaulting = 0x3E, // 撑杆僵尸
    seed_bucket_head = 0x3F,   // 桶头僵尸
    seed_ladder = 0x40,        // 梯子僵尸
    seed_digger = 0x41,        // 矿工僵尸
    seed_bungee = 0x42,        // 蹦极僵尸
    seed_football = 0x43,      // 橄榄球僵尸
    seed_balloon = 0x44,       // 气球僵尸
    seed_screen_door = 0x45,   // 铁门僵尸
    seed_zomboni = 0x46,       // 冰车僵尸
    seed_pogo = 0x47,          // 跳跳僵尸
    seed_dancing = 0x48,       // 舞王僵尸
    seed_gargantuar = 0x49,    // 巨人僵尸
    seed_imp = 0x4A            // 小鬼僵尸
};

enum class plant_status {
    idle = 0x0,                        // 空闲
    wait = 0x1,                        // 等待
    work = 0x2,                        // 工作/生效中
    squash_look = 0x3,                 // 倭瓜寻找目标
    squash_jump_up = 0x4,              // 倭瓜跳起
    squash_stop_in_the_air = 0x5,      // 倭瓜空中停留
    squash_jump_down = 0x6,            // 倭瓜下砸
    squash_crushed = 0x7,              // 倭瓜砸地完成
    grave_buster_land = 0x8,           // 墓碑吞噬者降落
    grave_buster_idle = 0x9,           // 墓碑吞噬者空闲
    chomper_bite_begin = 0xA,          // 大嘴花开始咬
    chomper_bite_success = 0xB,        // 大嘴花咬成功
    chomper_bite_fail = 0xC,           // 大嘴花咬失败
    chomper_chew = 0xD,                // 大嘴花嚼
    chomper_swallow = 0xE,             // 大嘴花咽
    potato_sprout_out = 0xF,           // 土豆雷破土
    potato_armed = 0x10,               // 土豆雷就绪
    potato_mashed = 0x11,              // 土豆雷爆炸完成
    spike_attack = 0x12,               // 地刺攻击
    spikeweed_attack2 = 0x13,          // 地刺攻击2 (TODO)
    scaredyshroom_scared = 0x14,       // 胆小菇害怕
    scaredyshroom_scared_idle = 0x15,  // 胆小菇躲起来待机中
    scaredyshroom_grow = 0x16,         // 胆小菇抬头
    sunshroom_small = 0x17,            // 阳光菇小形态
    sunshroom_grow = 0x18,             // 阳光菇中形态
    sunshroom_big = 0x19,              // 阳光菇大形态
    magnetshroom_working = 0x1A,       // 磁力菇吸取中
    magnetshroom_inactive_idle = 0x1B, // 磁力菇冷却中
    bowling_up = 0x1C,                 // 坚果保龄球向上弹
    bowling_down = 0x1D,               // 坚果保龄球向下弹
    cactus_short_idle = 0x1E,          // 仙人掌缩头
    cactus_grow_tall = 0x1F,           // 仙人掌伸长
    cactus_tall_idle = 0x20,           // 仙人掌拔高
    cactus_get_short = 0x21,           // 仙人掌变矮
    tangle_kelp_grab = 0x22,           // 缠绕水草抓取
    cob_cannon_unarmed_idle = 0x23,    // 玉米炮未装弹待机
    cob_cannon_charge = 0x24,          // 玉米炮装弹中
    cob_cannon_launch = 0x25,          // 玉米炮发射
    cob_cannon_armed_idle = 0x26,      // 玉米炮已装弹待机
    kernelpult_launch_butter = 0x27,   // 玉米投手投黄油
    umbrella_leaf_block = 0x28,        // 保护伞防御
    umbrella_leaf_shrink = 0x29,       // 保护伞收回
    imitater_morphing = 0x2A,          // 模仿者变形
    flower_pot_placed = 0x2F,          // 花盆放置
    lily_pad_placed = 0x30             // 睡莲放置
};

enum class attack_flags {
    ground = 0x1,
    flying_balloon = 0x2,
    lurking_snorkel = 0x4,
    animating_zombies = 0x10,
    dying_zombies = 0x20,
    digging_digger = 0x40,
    hypno_zombies = 0x80,
};

template <typename T>
unsigned int operator&(T a, attack_flags f) {
    return static_cast<unsigned int>(a) & static_cast<unsigned int>(f);
}

template <typename T>
unsigned int operator|(T a, attack_flags f) {
    return static_cast<unsigned int>(a) | static_cast<unsigned int>(f);
}

enum class plant_edible_status {
    visible_and_edible = 0,
    invisible_and_edible = 1,
    invisible_and_not_edible = 2
};

enum class plant_direction {
    left = 1,
    right = -1
};

class scene;

class plant {
public:
    int uuid;
    plant_type type;
    plant_status status;

    int x;
    int y;

    struct {
        int x;
        int y;
    } cannon;

    int hp;
    int max_hp;

    struct {
        int width;
        int height;
    } attack_box;

    unsigned int row;
    unsigned int col;

    unsigned int max_boot_delay;

    plant_direction direction;

    int target;

    plant_type imitater_target;

    struct {
        int status;
        int generate;
        int launch;
        int eaten;
        int awake;
        int effect;
        int dead;
        int blover_disappear;
    } countdown;

    object::reanim reanim;

    plant_edible_status edible;

    unsigned int threepeater_time_since_first_shot;

    struct {
        bool front;
        bool back;
    } split_pea_attack_flags;

    bool is_dead;
    bool is_smashed;
    bool is_sleeping;
    bool can_attack;
    bool ignore_garg_smash;
    bool ignore_jack_explode;
    
    struct explode_info {
        unsigned int from_upper;
        unsigned int from_same;
        unsigned int from_lower;
    };
    explode_info explode;

    static const std::array<unsigned int, 49> EFFECT_INTERVAL_TABLE;
    static const std::array<bool, 49> CAN_ATTACK_TABLE;
    static const std::array<unsigned int, 48> COST_TABLE;
    static const std::array<unsigned int, 48> CD_TABLE;

    bool is_squash_attacking() {
        return type == plant_type::squash && (
            status == plant_status::squash_stop_in_the_air ||
            status == plant_status::squash_jump_down ||
            status == plant_status::squash_crushed);
    }
         
    bool is_sun_plant() {
        return type == plant_type::sunflower ||
            type == plant_type::twin_sunflower ||
            type == plant_type::sunshroom;
    }

    bool is_freeable() {
        return is_dead;
    }

    void get_hit_box(rect &rect);
    void get_attack_box(rect& rect, bool is_alt_attack = false) const;

    unsigned int get_attack_flags(bool is_alt_attack = false) const;

    void set_sleep(bool is_sleep);

    void init_reanim();

    void set_reanim_frame(plant_reanim_name name);
    void set_reanim(plant_reanim_name name, reanim_type type, float fps);
    bool has_reanim(plant_reanim_name name);

    static const char* type_to_string(plant_type type);

    void to_json(scene& scene, rapidjson::Writer<rapidjson::StringBuffer>& writer);
    static const char* status_to_string(plant_status status);
};

};
