#pragma once
#include "../../../global/types.h"

void InitialiseHorseman(int16_t item_number);
void HorsemanControl(int16_t item_number);
void TriggerHorsemanRicochets(PHD_VECTOR* pos, int32_t yrot, int32_t num);
void InitialiseHorse(int16_t item_number);
