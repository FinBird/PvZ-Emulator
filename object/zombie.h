#pragma once
#include <array>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "common.h"
#include "zombie_reanim_data.h"

namespace pvz_emulator::object {

enum class zombie_type : std::int32_t {
    none = -1,
    zombie = 0x0,                // 普通僵尸
    flag = 0x1,                  // 摇旗僵尸
    conehead = 0x2,              // 路障僵尸
    pole_vaulting = 0x3,         // 撑杆僵尸
    buckethead = 0x4,            // 桶头僵尸
    newspaper = 0x5,             // 读报僵尸
    screendoor = 0x6,            // 铁门僵尸
    football = 0x7,              // 橄榄球僵尸
    dancing = 0x8,               // 舞王僵尸
    backup_dancer = 0x9,         // 伴舞僵尸
    ducky_tube = 0xa,            // 鸭子救生圈僵尸
    snorkel = 0xb,               // 潜水僵尸
    zomboni = 0xc,               // 冰车僵尸
    zombie_bobsled = 0xd,        // 雪橇小队 (TODO)
    dolphin_rider = 0xe,         // 海豚骑士僵尸
    jack_in_the_box = 0xf,       // 玩偶匣僵尸
    balloon = 0x10,              // 气球僵尸
    digger = 0x11,               // 矿工僵尸
    pogo = 0x12,                 // 跳跳僵尸
    yeti = 0x13,                 // 雪人僵尸
    bungee = 0x14,               // 蹦极僵尸
    ladder = 0x15,               // 梯子僵尸
    catapult = 0x16,             // 投石车僵尸
    gargantuar = 0x17,           // 巨人僵尸
    imp = 0x18,                  // 小鬼僵尸
    zombie_boss = 0x19,          // 僵尸博士 (TODO)
    zombie_pea_head = 0x1A,      // 豌豆僵尸 (TODO)
    zombie_wallnut_head = 0x1B,  // 坚果僵尸 (TODO)
    zombie_jalapeno_head = 0x1C, // 辣椒僵尸 (TODO)
    zombie_gatling_head = 0x1D,  // 机枪僵尸 (TODO)
    zombie_squash_head = 0x1E,   // 倭瓜僵尸 (TODO)
    zombie_tallnut_head = 0x1F,  // 高坚果僵尸 (TODO)
    giga_gargantuar = 0x20,      // 红眼巨人
};

enum class zombie_status : std::int32_t {
    walking = 0x0,                         // 步行
    dying = 0x1,                           // 正在死亡
    dying_from_instant_kill = 0x2,         // 瞬间爆发死亡 (如被炸)
    dying_from_lawnmower = 0x3,            // 被割草机杀
    bungee_target_drop = 0x4,              // 蹦极降落目标
    bungee_body_drop = 0x5,                // 蹦极身体降落
    bungee_idle_after_drop = 0x6,          // 蹦极降落后空闲
    bungee_grab = 0x7,                     // 蹦极抓取
    bungee_raise = 0x8,                    // 蹦极提升
    bungee_hit_ouchy = 9,                  // TODO:蹦极受伤
    bungee_idle = 0xa,                     // 蹦极待机
    pole_valuting_running = 0xb,           // 撑杆加速跑
    pole_valuting_jumping = 0xc,           // 撑杆跳跃中
    pole_vaulting_walking = 0xd,           // 撑杆跳后步行
    rising_from_ground = 0xe,              // 从地下钻出
    jackbox_walking = 0xf,                 // 匣子步行
    jackbox_pop = 0x10,                    // 匣子跳出
    bobsled_sliding = 0x11,                // 雪橇滑行
    bobsled_boarding = 0x12,               // 雪橇上车
    bobsled_crashing = 0x13,               // 雪橇撞毁
    pogo_with_stick = 0x14,                // 带杆跳跳
    pogo_idle_before_target = 0x15,        // 跳前待机
    pogo_high_bounce_2 = 0x16,             // 跳跳高跳2
    pogo_high_bounce_3 = 0x17,             // 跳跳高跳3
    pogo_high_bounce_4 = 0x18,             // 跳跳高跳4
    pogo_high_bounce_5 = 0x19,             // 跳跳高跳5
    pogo_high_bounce_6 = 0x1a,             // 跳跳高跳6
    pogo_jump_across = 0x1b,               // 跳跳越过
    pogo_forward_across2 = 0x1c,           // 跳跳向前越过2
    newspaper_walking = 0x1d,              // 读报步行
    newspaper_destroyed = 0x1e,            // 报纸被毁
    newspaper_running = 0x1f,              // 愤怒冲锋
    digger_dig = 0x20,                     // 矿工挖掘
    digger_drill = 0x21,                   // 矿工钻地
    digger_lost_dig = 0x22,                // 矿工丢失挖掘物
    digger_landing = 0x23,                 // 矿工降落/出土
    digger_dizzy = 0x24,                   // 矿工眩晕
    digger_walk_right = 0x25,              // 矿工向右走
    digger_walk_left = 0x26,               // 矿工向左走
    digger_idle = 0x27,                    // 矿工空闲
    dancing_moonwalk = 0x28,               // 舞王太空步
    dancing_point = 0x29,                  // 舞王指人
    dancing_wait_summoning = 0x2a,         // 舞王等待召唤
    dancing_summoning = 0x2b,              // 舞王召唤中
    dancing_walking = 0x2c,                // 舞王步行
    dancing_armrise1 = 0x2d,               // 舞王举手1
    dancing_armrise2 = 0x2e,               // 舞王举手2
    dancing_armrise3 = 0x2f,               // 舞王举手3
    dancing_armrise4 = 0x30,               // 舞王举手4
    dancing_armrise5 = 0x31,               // 舞王举手5
    backup_spawning = 0x32,                // 伴舞生成
    dolphin_walk_with_dolphin = 0x33,      // 海豚步行（陆地）
    dolphin_jump_in_pool = 0x34,           // 海豚入水
    dolphin_ride = 0x35,                   // 海豚骑乘
    dolphin_jump = 0x36,                   // 海豚跳跃
    dolphin_walk_in_pool = 0x37,           // 海豚水中步行
    dolphin_walk_without_dolphin = 0x38,   // 无海豚水中步行
    snorkel_walking = 0x39,                // 潜水步行
    snorkel_jump_in_the_pool = 0x3a,       // 潜水入水
    snorkel_swim = 0x3b,                   // 潜水游泳
    snorkel_up_to_eat = 0x3c,              // 潜水浮起吃
    snorkel_eat = 0x3d,                    // 潜水撕咬
    snorkel_finished_eat = 0x3e,           // 潜水吃完
    zombie_aquarium_accel = 0x3f,          // 僵尸水族馆加速
    zombie_aquarium_drift = 0x40,          // 僵尸水族馆漂流
    zombie_aquarium_back_and_forth = 0x41, // 僵尸水族馆来回游
    zombie_aquarium_bite = 0x42,           // 僵尸水族馆撕咬
    catapult_shoot = 0x43,                 // 投石车射击
    catapult_idle = 0x44,                  // 投石车空闲
    gargantuar_throw = 0x45,               // 巨人掷小鬼
    gargantuar_smash = 0x46,               // 巨人砸击
    imp_flying = 0x47,                     // 小鬼飞行
    imp_landing = 0x48,                    // 小鬼降落
    balloon_flying = 0x49,                 // 气球飞行
    balloon_falling = 0x4a,                // 气球坠落
    balloon_walking = 0x4b,                // 气球步行
    ladder_walking = 0x4c,                 // 梯子步行
    ladder_placing = 0x4d,                 // 梯子放置
    boss_enter = 0x4e,                     // 博士入场
    boss_idle = 0x4f,                      // 博士待机
    boss_spawning = 0x50,                  // 博士召唤
    boss_stomping = 0x51,                  // 博士踩踏
    boss_bungees_enter = 0x52,             // 博士蹦极入场
    boss_bungees_drop = 0x53,              // 博士蹦极降落
    boss_bungees_leave = 0x54,             // 博士蹦极离开
    boss_drop_rv = 0x55,                   // 博士扔房车
    boss_head_enter = 0x56,                // 博士头入场
    boss_head_idle_before_spit = 0x57,     // 博士喷吐前
    boss_head_idle_after_spit = 0x58,      // 博士喷吐后
    boss_head_spit = 0x59,                 // 博士喷吐
    boss_head_leave = 0x5a,                // 博士头离开
    yeti_escape = 0x5b,                    // 雪人逃跑
    squash_pre_launch = 0x5c,              // 倭瓜预发射（针对僵尸）
    squash_rising = 0x5d,                  // 倭瓜上升（针对僵尸）
    squash_falling = 0x5e,                 // 倭瓜下落（针对僵尸）
    squash_done_falling = 0x5f,            // 倭瓜下砸结束（针对僵尸）
};

enum class zombie_action : std::uint8_t {
    none = 0x0,
    entering_pool = 0x1,
    leaving_pool = 0x2,
    caught_by_kelp = 0x3,
    climbing_ladder = 0x6,
    falling = 0x7,
    fall_from_sky = 0x9
};

enum class zombie_accessories_type_1 : std::uint8_t {
    none = 0x0,
    roadcone = 0x1,
    bucket = 0x2,
    football_cap = 0x3,
    miner_hat = 0x4,
    redeyes = 0x5,
    headband = 0x6,
    bobsled = 0x7,
    wallnut = 0x8,
    tallnut = 0x9,
};

enum class zombie_accessories_type_2 : std::uint8_t {
    none = 0x0,
    screen_door = 0x1,
    newspaper = 0x2,
    ladder = 0x3,
};

enum class zombie_dance_cheat : std::uint8_t {
    none = 0x0,
    fast = 0x1,
    slow = 0x2,
};

class scene;

class zombie {
public:
    int uuid;
    struct {
        int arr[64];
        int size;
    } attempted_smashes;
    struct {
        int arr[4];
        int size;
    } ignored_smashes;
    struct {
        int arr[4];
        int size;
    } hit_by_ash;
    zombie_dance_cheat dance_cheat;

    zombie_type type;
    zombie_status status;
    zombie_action action;

    int int_x;
    int int_y;

    float x;
    float y;
    float dx;
    float dy;
    float d2y;

    object::reanim reanim;

    const float* _ground;

    unsigned int row;
    int bungee_col;
    int ladder_col;

    struct {
        int x;
        int y;
        int width;
        int height;
        int offset_x;
        int offset_y;
    } hit_box;

    struct {
        int x;
        int y;
        int width;
        int height;
    } attack_box;

    struct {
        unsigned int butter;
        unsigned int freeze;
        unsigned int slow;
        int action;
        int dead;
    } countdown;

    unsigned int spawn_wave;

    unsigned int time_since_ate_garlic;
    unsigned int time_since_spawn;

    int hp;
    unsigned int max_hp;

    struct {
        zombie_accessories_type_1 type;
        unsigned int hp;
        unsigned int max_hp;
    } accessory_1;

    struct {
        zombie_accessories_type_2 type;
        unsigned int hp;
        unsigned int max_hp;
    } accessory_2;

    int master_id;
    std::array<int, 4> partners;

    struct {
        unsigned int a;
        unsigned int b;
        unsigned int c;
    } garlic_tick;

    int bungee_target;

    union {
        unsigned int n_basketballs;
        unsigned int summon_countdown;
    } catapult_or_jackson;

    bool is_eating;
    bool is_dead;
    bool is_blown;
    bool is_not_dying;
    bool is_hypno;
    bool has_item_or_walk_left;
    bool is_in_water;
    bool has_balloon;
    bool has_eaten_garlic;

    bool has_reanim(zombie_reanim_name name) const;

    bool is_flying_or_falling() const {
        return status == zombie_status::balloon_flying ||
            status == zombie_status::balloon_falling;
    }

    float get_dx_from_ground() const;

    bool is_walk_right() const;

    bool has_death_status() const {
        return status == zombie_status::dying ||
            status == zombie_status::dying_from_instant_kill ||
            status == zombie_status::dying_from_lawnmower;
    }

    bool has_pogo_status() const {
        return status >= zombie_status::pogo_with_stick &&
            static_cast<int>(status) <= static_cast<int>(zombie_status::pogo_forward_across2);
    }

    bool can_be_slowed() const;

    bool can_be_freezed() const;

    bool is_freeable() const {
        return is_dead;
    }

    void get_attack_box(rect &rect) const;
    void get_hit_box(rect &rect) const;

    float get_height_bias() const;

    void init_reanim();

    void set_reanim_frame(zombie_reanim_name name);

    void to_json(scene& s, rapidjson::Writer<rapidjson::StringBuffer>& writer);

    static const char* status_to_string(zombie_status status);
    static const char* type_to_string(zombie_type type);

    static const char*
    accessories_type_to_string(zombie_accessories_type_1 type);

    static const char*
    accessories_type_to_string(zombie_accessories_type_2 type);
    };

};
