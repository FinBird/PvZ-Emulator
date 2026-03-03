#pragma once
#include <string>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace pvz_emulator::object {

int get_uuid();

struct rect {
    int x;
    int y;
    int width;
    int height;

    bool is_overlap_with_circle(int px, int py, int r);

    float get_overlap_len(const rect& r);

    void to_json(rapidjson::Writer<rapidjson::StringBuffer>& writer);
};

enum class reanim_type {
    repeat = 0,
    once = 1
};

struct reanim_frame_status {
    float frame_progress;
    unsigned int frame;
    unsigned int next_frame;
};

struct reanim {
    float fps;
    float prev_fps;
    unsigned int begin_frame;
    unsigned int n_frames;
    unsigned int n_repeated;
    float progress;
    float prev_progress;
    reanim_type type;

    bool is_in_progress(double progress);

    void get_frame_status(reanim_frame_status& rfs) const;

    void to_json(rapidjson::Writer<rapidjson::StringBuffer>& writer);
};

enum class damage_flags : std::uint8_t {
  none = 0x0,
  bypasses_shield = 0x1,             // 无视盾牌
  damage_hits_shield_and_body = 0x2, // 伤害同时作用于盾牌和身体
  damage_freeze = 0x4,               // 冰冻伤害
  no_flash = 0x8,                    // 不产生闪烁
  no_leave_body = 0x10,              // 不留下尸体
  spike = 0x20,                      // 地刺类伤害
};

} // namespace pvz_emulator::object

template <typename T>
unsigned int operator&(T a, pvz_emulator::object::damage_flags f) {
  return static_cast<unsigned int>(a) & static_cast<unsigned int>(f);
}

template <typename T>
unsigned int operator|(T a, pvz_emulator::object::damage_flags f) {
  return static_cast<unsigned int>(a) | static_cast<unsigned int>(f);

}