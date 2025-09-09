#pragma once
#include "../global/types.h"

void CreatureDie(int16_t item_number, bool explode);
void InitialiseCreature(int16_t item_number);
int32_t CreatureActive(int16_t item_number);
void CreatureAIInfo(ITEM_INFO* item, AI_INFO* info);
int32_t SearchLOT(LOT_INFO* LOT, int32_t expansion);
int32_t UpdateLOT(LOT_INFO* LOT, int32_t expansion);
void TargetBox(LOT_INFO* LOT, int16_t box_number);
int32_t EscapeBox(ITEM_INFO* item, ITEM_INFO* enemy, int16_t box_number);
int32_t ValidBox(ITEM_INFO* item, int16_t zone_number, int16_t box_number);
int32_t StalkBox(ITEM_INFO* item, ITEM_INFO* enemy, int16_t box_number);
target_type CalculateTarget(PHD_VECTOR* target, ITEM_INFO* item, LOT_INFO* LOT);
void CreatureMood(ITEM_INFO* item, AI_INFO* info, bool violent);
void GetCreatureMood(ITEM_INFO* item, AI_INFO* info, bool violent);
int32_t CreatureCreature(int16_t item_number);
int32_t BadFloor(int32_t x, int32_t y, int32_t z, int32_t box_height, int32_t next_height, int16_t room_number, LOT_INFO* LOT);
int32_t CreatureAnimation(int16_t item_number, int16_t angle, int16_t tilt);
int16_t CreatureTurn(ITEM_INFO* item, int16_t maximum_turn);
void CreatureTilt(ITEM_INFO* item, int16_t angle);
void CreatureJoint(ITEM_INFO* item, int16_t joint, int16_t required);
void CreatureFloat(int16_t item_number);
void CreatureUnderwater(ITEM_INFO* item, int32_t depth);
int16_t CreatureEffect(ITEM_INFO* item, BITE_INFO* bite, int16_t(*generate)(int32_t x, int32_t y, int32_t z, int16_t speed, int16_t yrot, int16_t room_number));
int16_t CreatureEffectT(ITEM_INFO* item, BITE_INFO* bite, int16_t damage, int16_t angle,
                        int16_t(*generate)(int32_t x, int32_t y, int32_t z, int16_t damage, int16_t angle, int16_t room_number));
int32_t CreatureVault(int16_t item_number, int16_t angle, int32_t vault, int32_t shift);
void CreatureKill(ITEM_INFO* item, int16_t kill_anim, int16_t kill_state, int16_t lara_anim);
void AlertAllGuards(int16_t item_number);
void AlertNearbyGuards(ITEM_INFO* item);
int16_t AIGuard(CREATURE_INFO* creature);
void FindAITargetObject(CREATURE_INFO* creature, int16_t obj_num);
void GetAITarget(CREATURE_INFO* creature);
int16_t SameZone(CREATURE_INFO* creature, ITEM_INFO* target_item);
void CreatureYRot(PHD_3DPOS* srcpos, int16_t angle, int16_t angadd);
int32_t MoveCreature3DPos(PHD_3DPOS* srcpos, PHD_3DPOS* destpos, int32_t velocity, int16_t angdif, int32_t angadd);

extern BOX_INFO* boxes;
extern uint16_t* overlap;
extern int16_t* ground_zone[5][2];
extern int32_t num_boxes;

#define BOX_CLIP_LEFT   1
#define BOX_CLIP_RIGHT  2
#define BOX_CLIP_TOP    4
#define BOX_CLIP_BOTTOM 8
#define BOX_CLIP_ALL (BOX_CLIP_LEFT | BOX_CLIP_RIGHT | BOX_CLIP_TOP | BOX_CLIP_BOTTOM)
#define BOX_CLIP_SECONDARY 16

#define NO_BOX          0x7ff
#define BOX_NUMBER      0x7ff

#define BLOCKABLE       0x8000
#define BLOCKED         0x4000
#define OVERLAP_INDEX   0x3fff

#define SEARCH_NUMBER   0x7fff
#define BLOCKED_SEARCH  0x8000

#define JUMP_BIT        0x800
#define MONKEY_BIT      0x2000

#define BOX_END_BIT     0x8000

#define MAX_EXPANSION   5

#define FRONT_ARC       0x4000

#define NO_FLYING       0

#define CLIP_LEFT   1
#define CLIP_RIGHT  2
#define CLIP_TOP    4
#define CLIP_BOTTOM 8
#define ALL_CLIP (BOX_CLIP_LEFT | BOX_CLIP_RIGHT | BOX_CLIP_TOP | BOX_CLIP_BOTTOM)
#define CLIP_SECONDARY 16
#define SECONDARY_CLIP BOX_CLIP_SECONDARY

#define NAVIGATION_GOAL_RADIUS (HALF_BLOCK_SIZE + HALF_CLICK_SIZE)
#define NAVIGATION_AVOIDENCE_TURN 0x800