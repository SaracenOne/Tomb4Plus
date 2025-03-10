#include "../../tomb4/pch.h"

#include "../control.h"
#include "../lara.h"
#include "../gameflow.h"

#include "trng.h"
#include "trng_condition.h"
#include "trng_flipeffect.h"
#include "trng_extra_state.h"
#include "trng_script_parser.h"
#include "trng_triggergroup.h"

void NGExecuteOrganizer(int32_t organizer_id) {
	NG_ORGANIZER* organizer = &ng_levels[gfCurrentLevel].records->organizer_table[organizer_id].record;
	int32_t record_id = ng_levels[gfCurrentLevel].records->organizer_table[organizer_id].record_id;

	bool global_trigger_condition_passed = false;

	for (uint32_t i = 0; i < organizer->appointment_count; i++) {
		if (ng_organizer_states[record_id].current_tick == organizer->appointments[i].time) {
			NGTriggerGroupFunction(organizer->appointments[i].trigger_group, TRIGGER_GROUP_EXECUTION_MULTIPLE);
			if (i == organizer->appointment_count - 1) {
				// FO_LOOP
				if (organizer->flags & 0x02) {
					ng_organizer_states[record_id].current_tick = -1;
				} else {
					ng_organizer_states[record_id].is_enabled = false;
				}
			}
		}
	}

	ng_organizer_states[record_id].current_tick += 1;
}

void NGProcessOrganizers() {
	if (ng_levels[gfCurrentLevel].records) {
		int32_t organizer_count = ng_levels[gfCurrentLevel].records->organizer_count;
		for (int32_t i = 0; i < organizer_count; i++) {
			int32_t record_id = ng_levels[gfCurrentLevel].records->organizer_table[i].record_id;

			if (ng_organizer_states[record_id].is_enabled) {
				NGExecuteOrganizer(i);
			}
		}
	}
}