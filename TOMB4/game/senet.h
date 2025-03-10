#pragma once
#include "../global/types.h"

void InitialiseSenet(int16_t item_number);
void MakeMove(int32_t piece, int32_t displacement);
void SenetControl(int16_t item_number);
int32_t CheckSenetWinner(int32_t won);
void InitialiseGameStix(int16_t item_number);
void ThrowSticks(ITEM_INFO* item);
void GameStixControl(int16_t item_number);
void GameStixCollision(int16_t item_number, ITEM_INFO* l, COLL_INFO* coll);
void ShockwaveExplosion(ITEM_INFO* item, uint32_t col, int32_t speed);
void ControlGodHead(int16_t item_number);
void DrawGodHead(ITEM_INFO* item);

#define SENET_ITEM_MID 3
#define SENET_ITEM_COUNT 6
#define SENET_BOARD_COUNT 17

extern int32_t SenetTargetX;
extern int32_t SenetTargetZ;
extern int16_t senet_item[SENET_ITEM_COUNT];
extern int8_t piece_moving;
extern int8_t last_throw;
extern int8_t senet_board[SENET_BOARD_COUNT];
extern int8_t senet_piece[SENET_ITEM_COUNT];
