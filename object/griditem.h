#pragma once
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

namespace pvz_emulator::object {

enum class griditem_type {
    none = 0x0,           // 无
    grave = 0x1,          // 墓碑
    crater = 0x2,         // 弹坑
    ladder = 0x3,         // 梯子
    portal_circle = 0x4,  // 圆形传送门 (TODO)
    portal_square = 0x5,  // 方形传送门 (TODO)
    brain = 0x6,          // 脑子 (TODO)
    scary_pot = 0x7,      // ??? (TODO)
    squirrel = 0x8,       // ??? (TODO)
    zen_tool = 0x9,       // ??? (TODO)
    stinky = 0xA,         // 蜗牛 (TODO)
    rake = 0xB,           // 钉耙 (TODO)
    i_zombie_brain = 0xC, // IZombie 脑子 (TODO)
};

class scene;

class griditem {
public:
    griditem_type type;
    unsigned int col;
    unsigned int row;
    int countdown;
    bool is_disappeared;

    bool is_freeable() {
        return is_disappeared;
    }

    void to_json(
        scene& scene,
        rapidjson::Writer<rapidjson::StringBuffer>& writer);
};

}
