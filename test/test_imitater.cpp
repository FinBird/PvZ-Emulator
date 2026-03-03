#include "catch_amalgamated.hpp"
#include "object/plant.h"
#include "object/zombie.h"
#include "system/damage.h"
#include "world.h"


using namespace pvz_emulator;
using namespace pvz_emulator::object;

class ImitaterTest {
protected:
  ImitaterTest() {
    w = std::make_unique<world>(scene_type::day);
    w->reset();
    w->scene.stop_spawn = true;
  }

  std::unique_ptr<world> w;

  plant *find_plant(plant_type type) {
    for (auto &p : w->scene.plants) {
      if (!p.is_dead && p.type == type)
        return &p;
    }
    return nullptr;
  }
};

TEST_CASE_METHOD(ImitaterTest, "InterruptedByGargantuarSmash",
                 "[ImitaterTest]") {
  w->scene.cards[0].type = plant_type::imitater;
  w->scene.cards[0].imitater_type = plant_type::repeater;
  w->plant(0, 2, 2);

  plant *imitater = find_plant(plant_type::imitater);
  REQUIRE(imitater != nullptr);

  system::damage dmg_sys(w->scene);
  dmg_sys.set_smashed(*imitater);

  CHECK(imitater->is_smashed);
  CHECK(imitater->countdown.dead == 500);

  for (int i = 0; i < 600; ++i)
    w->update();

  CHECK(imitater->is_dead);
  CHECK(find_plant(plant_type::repeater) == nullptr);
}

TEST_CASE_METHOD(ImitaterTest, "MorphingStageIsNotEdible", "[ImitaterTest]") {
  w->scene.cards[0].type = plant_type::imitater;
  w->scene.cards[0].imitater_type = plant_type::wallnut;
  w->plant(0, 2, 2);

  plant *imitater = find_plant(plant_type::imitater);
  imitater->countdown.status = 0;
  w->update();

  CHECK(imitater->status == plant_status::imitater_morphing);
  CHECK(imitater->edible == plant_edible_status::invisible_and_not_edible);
}

TEST_CASE_METHOD(ImitaterTest, "InheritsFullHealthAfterMorph",
                 "[ImitaterTest]") {
  w->scene.cards[0].type = plant_type::imitater;
  w->scene.cards[0].imitater_type = plant_type::tallnut;
  w->plant(0, 2, 2);

  plant *imitater = find_plant(plant_type::imitater);
  imitater->hp = 10;
  imitater->countdown.status = 0;

  int safety = 0;
  while (find_plant(plant_type::imitater) != nullptr && safety < 324) {
    w->update();
    safety++;
  }

  plant *tallnut = find_plant(plant_type::tallnut);
  REQUIRE(tallnut != nullptr);
  CHECK(tallnut->hp == 8000);
}

TEST_CASE_METHOD(ImitaterTest, "SmashedDuringMorph", "[ImitaterTest]") {
  w->scene.cards[0].type = plant_type::imitater;
  w->scene.cards[0].imitater_type = plant_type::repeater;
  w->plant(0, 2, 2);

  plant *imitater = nullptr;
  for (auto &p : w->scene.plants)
    if (p.type == plant_type::imitater)
      imitater = &p;
  REQUIRE(imitater != nullptr);

  imitater->countdown.status = 0;
  w->update();
  REQUIRE(imitater->status == plant_status::imitater_morphing);

  system::damage dmg(w->scene);
  dmg.set_smashed(*imitater);

  for (int i = 0; i < 50; ++i)
    w->update();

  CHECK((imitater->is_dead || imitater->is_smashed));

  bool found_repeater = false;
  for (auto &p : w->scene.plants)
    if (p.type == plant_type::repeater)
      found_repeater = true;

  CHECK_FALSE(found_repeater);
}

TEST_CASE_METHOD(ImitaterTest, "EatenDuringMorph", "[ImitaterTest]") {
  w->scene.cards[0].type = plant_type::imitater;
  w->scene.cards[0].imitater_type = plant_type::wallnut;
  w->plant(0, 2, 5);

  plant *imitater = find_plant(plant_type::imitater);
  REQUIRE(imitater != nullptr);
  REQUIRE(imitater->countdown.status > 0);
  REQUIRE(imitater->status == plant_status::idle);

  auto &z = w->zombie_factory.create(zombie_type::zombie, 2);
  z.x = imitater->x;
  z.int_x = (int)z.x;

  imitater->hp = 4;
  for (int i = 0; i < 20; ++i)
    w->update();

  CHECK(imitater->is_dead);
  CHECK(find_plant(plant_type::wallnut) == nullptr);
}

TEST_CASE_METHOD(ImitaterTest, "NotTargetableAfterMorph", "[ImitaterTest]") {
  w->scene.cards[0].type = plant_type::imitater;
  w->scene.cards[0].imitater_type = plant_type::pea_shooter;
  w->plant(0, 2, 2);

  int ticks = 0;
  while (ticks < 500) {
    w->update();
    bool found_pea = false;
    for (auto &p : w->scene.plants)
      if (p.type == plant_type::pea_shooter)
        found_pea = true;
    if (found_pea)
      break;
    ticks++;
  }

  auto &gs = w->scene.plant_map[2][2];
  REQUIRE(gs.content != nullptr);
  CHECK(gs.content->type == plant_type::pea_shooter);
}