#pragma once
#include "../../../global/types.h"

extern void lara_as_trpose(ITEM_INFO* item, COLL_INFO* coll);
extern void lara_as_trwalk(ITEM_INFO* item, COLL_INFO* coll);
extern void lara_as_trfall(ITEM_INFO* item, COLL_INFO* coll);

extern void InitialiseTightRope(int16_t item_number);
extern void TightRopeCollision(int16_t item_num, ITEM_INFO* l, COLL_INFO* coll);

struct LARA_TIGHTROPE_EXTRASTATE {
	uint8_t TightRopeOnCount;
	uint8_t TightRopeOff;
	uint8_t TightRopeFall;
};

extern LARA_TIGHTROPE_EXTRASTATE lara_tightrope_extrastate;

void SetupLaraTightropeExtraState();