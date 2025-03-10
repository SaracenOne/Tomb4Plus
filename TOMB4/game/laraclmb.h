#pragma once
#include "../global/types.h"

void lara_as_climbstnc(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbleft(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbleft(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbright(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbright(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbing(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbdown(ITEM_INFO* item, COLL_INFO* coll);
void lara_as_climbend(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbstnc(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbing(ITEM_INFO* item, COLL_INFO* coll);
void lara_col_climbdown(ITEM_INFO* item, COLL_INFO* coll);
int16_t GetClimbTrigger(int32_t x, int32_t y, int32_t z, int16_t room_number);
int32_t LaraTestClimb(int32_t x, int32_t y, int32_t z, int32_t xfront, int32_t zfront, int32_t item_height, int16_t item_room, int32_t* shift);
int32_t LaraTestClimbPos(ITEM_INFO* item, int32_t front, int32_t right, int32_t origin, int32_t height, int32_t* shift);
int32_t LaraTestClimbUpPos(ITEM_INFO* item, int32_t front, int32_t right, int32_t* shift, int32_t* ledge);
int32_t LaraCheckForLetGo(ITEM_INFO* item, COLL_INFO* coll);
int32_t LaraClimbLeftCornerTest(ITEM_INFO* item, COLL_INFO* coll);
int32_t LaraClimbRightCornerTest(ITEM_INFO* item, COLL_INFO* coll);
void LaraDoClimbLeftRight(ITEM_INFO* item, COLL_INFO* coll, int32_t result, int32_t shift);
