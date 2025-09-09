#include "../../tomb4/pch.h"

#include "trng.h"
#include "trng_arithmetic.h"
#include "trng_extra_state.h"

#include "../lara.h"
#include "../gameflow.h"
#include "../savegame.h"
#include "../control.h"


// TODO: Investigate whether numeric operations operate as signed or unsigned.
uint8_t NGNumericOperation8(NGNumericOperationType number_operation_type, uint8_t variable, uint32_t value) {
	switch (number_operation_type) {
		case NG_SET: {
			return value;
		}
		case NG_ADD: {
			return variable + value;
		}
		case NG_SUBTRACT: {
			return variable - value;
		}
		case NG_MULTIPLY: {
			return variable * value;
		}
		case NG_DIVIDE: {
			if (value != 0) {
				return variable / value;
			} else {
				return 0;
			}
		}
		case NG_BIT_SET: {
			return variable | (1 << value);
		}
		case NG_BIT_CLEAR: {
			return variable & ~(1 << value);
		}
		case NG_INVERT_SIGN: {
			return (uint8_t)(-(int8_t)value);
		}
	}

	return 0;
}

uint16_t NGNumericOperation16(NGNumericOperationType number_operation_type, uint16_t variable, uint32_t value) {
	switch (number_operation_type) {
		case NG_SET: {
			return value;
			break;
		}
		case NG_ADD: {
			return variable + value;
			break;
		}
		case NG_SUBTRACT: {
			return variable - value;
			break;
		}
		case NG_MULTIPLY: {
			return variable * value;
			break;
		}
		case NG_DIVIDE: {
			return variable / value;
			break;
		}
		case NG_BIT_SET: {
			return variable | (1 << value);
		}
		case NG_BIT_CLEAR: {
			return variable & ~(1 << value);
		}
		case NG_INVERT_SIGN: {
			return (uint16_t)(-(int16_t)value);
		}
	}

	return 0;
}

uint32_t NGNumericOperation32(NGNumericOperationType number_operation_type, uint32_t variable, uint32_t value) {
	switch (number_operation_type) {
		case NG_SET: {
			return value;
			break;
		}
		case NG_ADD: {
			return variable + value;
			break;
		}
		case NG_SUBTRACT: {
			return variable - value;
			break;
		}
		case NG_MULTIPLY: {
			return variable * value;
			break;
		}
		case NG_DIVIDE: {
			return variable / value;
			break;
		}
		case NG_BIT_SET: {
			return variable | (1 << value);
		}
		case NG_BIT_CLEAR: {
			return variable & ~(1 << value);
		}
		case NG_INVERT_SIGN: {
			return (uint32_t)(-(int32_t)value);
		}
	}

	return 0;
}

void NGNumericOperation(NGNumericOperationType number_operation, uint32_t variable, uint32_t value) {
	switch (variable) {
		case 0xffffffff: {
			ng_current_value = NGNumericOperation8(number_operation, ng_current_value, value);
			break;
		}
		case 0xffff: {
			ng_current_value = NGNumericOperation8(number_operation, ng_current_value, value);
			break;
		}
		case 0xff: {
			ng_current_value = NGNumericOperation8(number_operation, ng_current_value, value);
			break;
		}

		/* Globals */

		// Global Alfa Byte
		case 0x00: {
			uint8_t ng_global_alfa_1 = (ng_global_alfa & 0xff);
			ng_global_alfa_1 = NGNumericOperation8(number_operation, ng_global_alfa_1, value);
			ng_global_alfa = (ng_global_alfa & ~0xff) | (((int32_t)ng_global_alfa_1)) & 0xff;
			break;
		}
		case 0x01: {
			uint8_t ng_global_alfa_2 = (ng_global_alfa >> 8) & 0xff;
			ng_global_alfa_2 = NGNumericOperation8(number_operation, ng_global_alfa_2, value);
			ng_global_alfa = (ng_global_alfa & ~0xff00) | (((int32_t)ng_global_alfa_2) << 8) & 0xff00;
			break;
		}
		case 0x02: {
			uint8_t ng_global_alfa_3 = (ng_global_alfa >> 16) & 0xff;
			ng_global_alfa_3 = NGNumericOperation8(number_operation, ng_global_alfa_3, value);
			ng_global_alfa = (ng_global_alfa & ~0xff0000) | (((int32_t)ng_global_alfa_3) << 16) & 0xff0000;
			break;
		}
		case 0x03: {
			uint8_t ng_global_alfa_4 = (ng_global_alfa >> 24) & 0xff;
			ng_global_alfa_4 = NGNumericOperation8(number_operation, ng_global_alfa_4, value);
			ng_global_alfa = (ng_global_alfa & ~0xff000000) | (((int32_t)ng_global_alfa_4) << 24) & 0xff000000;
			break;
		}
		// Global Beta Byte
		case 0x04: {
			uint8_t ng_global_beta_1 = (ng_global_beta & 0xff);
			ng_global_beta_1 = NGNumericOperation8(number_operation, ng_global_beta_1, value);
			ng_global_beta = (ng_global_beta & ~0xff) | (((int32_t)ng_global_beta_1)) & 0xff;
			break;
		}
		case 0x05: {
			uint8_t ng_global_beta_2 = (ng_global_beta >> 8) & 0xff;
			ng_global_beta_2 = NGNumericOperation8(number_operation, ng_global_beta_2, value);
			ng_global_beta = (ng_global_beta & ~0xff00) | (((int32_t)ng_global_beta_2) << 8) & 0xff00;
			break;
		}
		case 0x06: {
			uint8_t ng_global_beta_3 = (ng_global_beta >> 16) & 0xff;
			ng_global_beta_3 = NGNumericOperation8(number_operation, ng_global_beta_3, value);
			ng_global_beta = (ng_global_beta & ~0xff0000) | (((int32_t)ng_global_beta_3) << 16) & 0xff0000;
			break;
		}
		case 0x07: {
			uint8_t ng_global_beta_4 = (ng_global_beta >> 24) & 0xff;
			ng_global_beta_4 = NGNumericOperation8(number_operation, ng_global_beta_4, value);
			ng_global_beta = (ng_global_beta & ~0xff000000) | (((int32_t)ng_global_beta_4) << 24) & 0xff000000;
			break;
		}
		// Global Delta Byte
		case 0x08: {
			uint8_t ng_global_delta_1 = (ng_global_delta & 0xff);
			ng_global_delta_1 = NGNumericOperation8(number_operation, ng_global_delta_1, value);
			ng_global_delta = (ng_global_delta & ~0xff) | (((int32_t)ng_global_delta_1)) & 0xff;
			break;
		}
		case 0x09: {
			uint8_t ng_global_delta_2 = (ng_global_delta >> 8) & 0xff;
			ng_global_delta_2 = NGNumericOperation8(number_operation, ng_global_delta_2, value);
			ng_global_delta = (ng_global_delta & ~0xff00) | (((int32_t)ng_global_delta_2) << 8) & 0xff00;
			break;
		}
		case 0x0a: {
			uint8_t ng_global_delta_3 = (ng_global_delta >> 16) & 0xff;
			ng_global_delta_3 = NGNumericOperation8(number_operation, ng_global_delta_3, value);
			ng_global_delta = (ng_global_delta & ~0xff0000) | (((int32_t)ng_global_delta_3) << 16) & 0xff0000;
			break;
		}
		case 0x0b: {
			uint8_t ng_global_delta_4 = (ng_global_delta >> 24) & 0xff;
			ng_global_delta_4 = NGNumericOperation8(number_operation, ng_global_delta_4, value);
			ng_global_delta = (ng_global_delta & ~0xff000000) | (((int32_t)ng_global_delta_4) << 24) & 0xff000000;
			break;
		}
		// Global 16 Alfa
		case 0x10: {
			uint16_t ng_global_16_alfa_1 = (ng_global_alfa & 0xffff);
			ng_global_16_alfa_1 = NGNumericOperation16(number_operation, ng_global_16_alfa_1, value);
			ng_global_alfa = (ng_global_alfa & ~0xffff) | (((int32_t)ng_global_16_alfa_1)) & 0xffff;
			break;
		}
		case 0x11: {
			uint16_t ng_global_16_alfa_2 = (ng_global_alfa & 0xffff0000);
			ng_global_16_alfa_2 = NGNumericOperation16(number_operation, ng_global_16_alfa_2, value);
			ng_global_alfa = (ng_global_alfa & ~0xffff0000) | (((int32_t)ng_global_16_alfa_2)) & 0xffff0000;
			break;
		}
		// Global 16 Beta
		case 0x12: {
			uint16_t ng_global_16_beta_1 = (ng_global_beta & 0xffff);
			ng_global_16_beta_1 = NGNumericOperation16(number_operation, ng_global_16_beta_1, value);
			ng_global_beta = (ng_global_beta & ~0xffff) | (((int32_t)ng_global_16_beta_1)) & 0xffff;
			break;
		}
		case 0x13: {
			uint16_t ng_global_16_beta_2 = (ng_global_beta & 0xffff0000);
			ng_global_16_beta_2 = NGNumericOperation16(number_operation, ng_global_16_beta_2, value);
			ng_global_beta = (ng_global_beta & ~0xffff0000) | (((int32_t)ng_global_16_beta_2)) & 0xffff0000;
			break;
		}
		// Global 16 Delta
		case 0x14: {
			uint16_t ng_global_16_delta_1 = (ng_global_delta & 0xffff);
			ng_global_16_delta_1 = NGNumericOperation16(number_operation, ng_global_16_delta_1, value);
			ng_global_delta = (ng_global_delta & ~0xffff) | (((int32_t)ng_global_16_delta_1)) & 0xffff;
			break;
		}
		case 0x15: {
			uint16_t ng_global_16_delta_2 = (ng_global_delta & 0xffff0000);
			ng_global_16_delta_2 = NGNumericOperation16(number_operation, ng_global_16_delta_2, value);
			ng_global_delta = (ng_global_delta & ~0xffff0000) | (((int32_t)ng_global_16_delta_2)) & 0xffff0000;
			break;
		}

		// Global 32 Alfa
		case 0x30: {
			ng_global_alfa = NGNumericOperation32(number_operation, ng_global_alfa, value);
			break;
		}
		// Global 32 Beta
		case 0x31: {
			ng_global_beta = NGNumericOperation32(number_operation, ng_global_beta, value);
			break;
		}
		// Global 32 Delta
		case 0x32: {
			ng_global_delta = NGNumericOperation32(number_operation, ng_global_delta, value);
			break;
		}
		// Global 32 Timer
		case 0x33: {
			ng_global_timer = NGNumericOperation32(number_operation, ng_global_timer, value);
			break;
		}
		// Global Last Input Number
		case 0x35: {
			ng_last_input_number = NGNumericOperation32(number_operation, ng_global_timer, value);
			break;
		}

		/* Locals */

		// Local Alfa Byte
		case 0x40: {
			uint8_t ng_local_alfa_1 = (ng_local_alfa & 0xff);
			ng_local_alfa_1 = NGNumericOperation8(number_operation, ng_local_alfa_1, value);
			ng_local_alfa = (ng_local_alfa & ~0xff) | (((int32_t)ng_local_alfa_1)) & 0xff;
			break;
		}
		case 0x41: {
			uint8_t ng_local_alfa_2 = (ng_local_alfa >> 8) & 0xff;
			ng_local_alfa_2 = NGNumericOperation8(number_operation, ng_local_alfa_2, value);
			ng_local_alfa = (ng_local_alfa & ~0xff00) | (((int32_t)ng_local_alfa_2) << 8) & 0xff00;
			break;
		}
		case 0x42: {
			uint8_t ng_local_alfa_3 = (ng_local_alfa >> 16) & 0xff;
			ng_local_alfa_3 = NGNumericOperation8(number_operation, ng_local_alfa_3, value);
			ng_local_alfa = (ng_local_alfa & ~0xff0000) | (((int32_t)ng_local_alfa_3) << 16) & 0xff0000;
			break;
		}
		case 0x43: {
			uint8_t ng_local_alfa_4 = (ng_local_alfa >> 24) & 0xff;
			ng_local_alfa_4 = NGNumericOperation8(number_operation, ng_local_alfa_4, value);
			ng_local_alfa = (ng_local_alfa & ~0xff000000) | (((int32_t)ng_local_alfa_4) << 24) & 0xff000000;
			break;
		}
		// Local Beta Byte
		case 0x44: {
			uint8_t ng_local_beta_1 = (ng_local_beta & 0xff);
			ng_local_beta_1 = NGNumericOperation8(number_operation, ng_local_beta_1, value);
			ng_local_beta = (ng_local_beta & ~0xff) | (((int32_t)ng_local_beta_1)) & 0xff;
			break;
		}
		case 0x45: {
			uint8_t ng_local_beta_2 = (ng_local_beta >> 8) & 0xff;
			ng_local_beta_2 = NGNumericOperation8(number_operation, ng_local_beta_2, value);
			ng_local_beta = (ng_local_beta & ~0xff00) | (((int32_t)ng_local_beta_2) << 8) & 0xff00;
			break;
		}
		case 0x46: {
			uint8_t ng_local_beta_3 = (ng_local_beta >> 16) & 0xff;
			ng_local_beta_3 = NGNumericOperation8(number_operation, ng_local_beta_3, value);
			ng_local_beta = (ng_local_beta & ~0xff0000) | (((int32_t)ng_local_beta_3) << 16) & 0xff0000;
			break;
		}
		case 0x47: {
			uint8_t ng_local_beta_4 = (ng_local_beta >> 24) & 0xff;
			ng_local_beta_4 = NGNumericOperation8(number_operation, ng_local_beta_4, value);
			ng_local_beta = (ng_local_beta & ~0xff000000) | (((int32_t)ng_local_beta_4) << 24) & 0xff000000;
			break;
		}
		// Local Delta Byte
		case 0x48: {
			uint8_t ng_local_delta_1 = (ng_local_delta & 0xff);
			ng_local_delta_1 = NGNumericOperation8(number_operation, ng_local_delta_1, value);
			ng_local_delta = (ng_local_delta & ~0xff) | (((int32_t)ng_local_delta_1)) & 0xff;
			break;
		}
		case 0x49: {
			uint8_t ng_local_delta_2 = (ng_local_delta >> 8) & 0xff;
			ng_local_delta_2 = NGNumericOperation8(number_operation, ng_local_delta_2, value);
			ng_local_delta = (ng_local_delta & ~0xff00) | (((int32_t)ng_local_delta_2) << 8) & 0xff00;
			break;
		}
		case 0x4a: {
			uint8_t ng_local_delta_3 = (ng_local_delta >> 16) & 0xff;
			ng_local_delta_3 = NGNumericOperation8(number_operation, ng_local_delta_3, value);
			ng_local_delta = (ng_local_delta & ~0xff0000) | (((int32_t)ng_local_delta_3) << 16) & 0xff0000;
			break;
		}
		case 0x4b: {
			uint8_t ng_local_delta_4 = (ng_local_delta >> 24) & 0xff;
			ng_local_delta_4 = NGNumericOperation8(number_operation, ng_local_delta_4, value);
			ng_local_delta = (ng_local_delta & ~0xff000000) | (((int32_t)ng_local_delta_4) << 24) & 0xff000000;
			break;
		}
		// Local 16 Alfa
		case 0x50: {
			uint16_t ng_local_16_alfa_1 = (ng_local_alfa & 0xffff);
			ng_local_16_alfa_1 = NGNumericOperation16(number_operation, ng_local_16_alfa_1, value);
			ng_local_alfa = (ng_local_alfa & ~0xffff) | (((int32_t)ng_local_16_alfa_1)) & 0xffff;
			break;
		}
		case 0x51: {
			uint16_t ng_local_16_alfa_2 = (ng_local_alfa & 0xffff0000);
			ng_local_16_alfa_2 = NGNumericOperation16(number_operation, ng_local_16_alfa_2, value);
			ng_local_alfa = (ng_local_alfa & ~0xffff0000) | (((int32_t)ng_local_16_alfa_2)) & 0xffff0000;
			break;
		}
		// Local 16 Beta
		case 0x52: {
			uint16_t ng_local_16_beta_1 = (ng_local_beta & 0xffff);
			ng_local_16_beta_1 = NGNumericOperation16(number_operation, ng_local_16_beta_1, value);
			ng_local_beta = (ng_local_beta & ~0xffff) | (((int32_t)ng_local_16_beta_1)) & 0xffff;
			break;
		}
		case 0x53: {
			uint16_t ng_local_16_beta_2 = (ng_local_beta & 0xffff0000);
			ng_local_16_beta_2 = NGNumericOperation16(number_operation, ng_local_16_beta_2, value);
			ng_local_beta = (ng_local_beta & ~0xffff0000) | (((int32_t)ng_local_16_beta_2)) & 0xffff0000;
			break;
		}
		// Local 16 Delta
		case 0x54: {
			uint16_t ng_local_16_delta_1 = (ng_local_delta & 0xffff);
			ng_local_16_delta_1 = NGNumericOperation16(number_operation, ng_local_16_delta_1, value);
			ng_local_delta = (ng_local_delta & ~0xffff) | (((int32_t)ng_local_16_delta_1)) & 0xffff;
			break;
		}
		case 0x55: {
			uint16_t ng_local_16_delta_2 = (ng_local_delta & 0xffff0000);
			ng_local_16_delta_2 = NGNumericOperation16(number_operation, ng_local_16_delta_2, value);
			ng_local_delta = (ng_local_delta & ~0xffff0000) | (((int32_t)ng_local_16_delta_2)) & 0xffff0000;
			break;
		}

		// Local 32 Alfa
		case 0x70: {
			ng_local_alfa = NGNumericOperation32(number_operation, ng_local_alfa, value);
			break;
		}
		// Local 32 Beta
		case 0x71: {
			ng_local_beta = NGNumericOperation32(number_operation, ng_local_beta, value);
			break;
		}
		// Local 32 Delta
		case 0x72: {
			ng_local_delta = NGNumericOperation32(number_operation, ng_local_delta, value);
			break;
		}
		// Local 32 Timer
		case 0x73: {
			ng_local_timer = NGNumericOperation32(number_operation, ng_local_timer, value);
			break;
		}
	}
}

int32_t NGNumericGetVariable(uint32_t variable) {
	switch (variable) {
		case 0xffffffff: {
			return ng_current_value;
		}
		case 0xffff: {
			return ng_current_value;
		}
		case 0xff: {
			return ng_current_value;
		}

		/* Globals */

		// Global Alfa Byte
		case 0x00: {
			uint8_t ng_global_alfa_1 = (ng_global_alfa & 0xff);
			return ng_global_alfa_1;
		}
		case 0x01: {
			uint8_t ng_global_alfa_2 = (ng_global_alfa >> 8) & 0xff;
			return ng_global_alfa_2;
		}
		case 0x02: {
			uint8_t ng_global_alfa_3 = (ng_global_alfa >> 16) & 0xff;
			return ng_global_alfa_3;
		}
		case 0x03: {
			uint8_t ng_global_alfa_4 = (ng_global_alfa >> 24) & 0xff;
			return ng_global_alfa_4;
		}
		// Global Beta Byte
		case 0x04: {
			uint8_t ng_global_beta_1 = (ng_global_beta & 0xff);
			return ng_global_beta_1;
		}
		case 0x05: {
			uint8_t ng_global_beta_2 = (ng_global_beta >> 8) & 0xff;
			return ng_global_beta_2;
		}
		case 0x06: {
			uint8_t ng_global_beta_3 = (ng_global_beta >> 16) & 0xff;
			return ng_global_beta_3;
		}
		case 0x07: {
			uint8_t ng_global_beta_4 = (ng_global_beta >> 24) & 0xff;
			return ng_global_beta_4;
		}
		// Global Delta Byte
		case 0x08: {
			uint8_t ng_global_delta_1 = (ng_global_delta & 0xff);
			return ng_global_delta_1;
		}
		case 0x09: {
			uint8_t ng_global_delta_2 = (ng_global_delta >> 8) & 0xff;
			return ng_global_delta_2;
		}
		case 0x0a: {
			uint8_t ng_global_delta_3 = (ng_global_delta >> 16) & 0xff;
			return ng_global_delta_3;
		}
		case 0x0b: {
			uint8_t ng_global_delta_4 = (ng_global_delta >> 24) & 0xff;
			return ng_global_delta_4;
		}
		// Global 16 Alfa
		case 0x10: {
			uint16_t ng_global_16_alfa_1 = (ng_global_alfa & 0xffff);
			return ng_global_16_alfa_1;
		}
		case 0x11: {
			uint16_t ng_global_16_alfa_2 = (ng_global_alfa & 0xffff0000);
			return ng_global_16_alfa_2;
		}
		// Global 16 Beta
		case 0x12: {
			uint16_t ng_global_16_beta_1 = (ng_global_beta & 0xffff);
			return ng_global_16_beta_1;
		}
		case 0x13: {
			uint16_t ng_global_16_beta_2 = (ng_global_beta & 0xffff0000);
			return ng_global_16_beta_2;
		}
		// Global 16 Delta
		case 0x14: {
			uint16_t ng_global_16_delta_1 = (ng_global_delta & 0xffff);
			return ng_global_16_delta_1;
		}
		case 0x15: {
			uint16_t ng_global_16_delta_2 = (ng_global_delta & 0xffff0000);
			return ng_global_16_delta_2;
		}

		// Global 32 Alfa
		case 0x30: {
			return ng_global_alfa;
		}
		// Global 32 Beta
		case 0x31: {
			return ng_global_beta;
		}
		// Global 32 Delta
		case 0x32: {
			return ng_global_delta;
		}
		// Global 32 Timer
		case 0x33: {
			return ng_global_timer;
		}
		// Global Last Input Number
		case 0x35: {
			return ng_last_input_number;
		}

		/* Locals */

		// Local Alfa Byte
		case 0x40: {
			uint8_t ng_local_alfa_1 = (ng_local_alfa & 0xff);
			return ng_local_alfa_1;
		}
		case 0x41: {
			uint8_t ng_local_alfa_2 = (ng_local_alfa >> 8) & 0xff;
			return ng_local_alfa_2;
		}
		case 0x42: {
			uint8_t ng_local_alfa_3 = (ng_local_alfa >> 16) & 0xff;
			return ng_local_alfa_3;
		}
		case 0x43: {
			uint8_t ng_local_alfa_4 = (ng_local_alfa >> 24) & 0xff;
			return ng_local_alfa_4;
		}
		// Local Beta Byte
		case 0x44: {
			uint8_t ng_local_beta_1 = (ng_local_beta & 0xff);
			return ng_local_beta_1;
		}
		case 0x45: {
			uint8_t ng_local_beta_2 = (ng_local_beta >> 8) & 0xff;
			return ng_local_beta_2;
		}
		case 0x46: {
			uint8_t ng_local_beta_3 = (ng_local_beta >> 16) & 0xff;
			return ng_local_beta_3;
		}
		case 0x47: {
			uint8_t ng_local_beta_4 = (ng_local_beta >> 24) & 0xff;
			return ng_local_beta_4;
		}
		// Local Delta Byte
		case 0x48: {
			uint8_t ng_local_delta_1 = (ng_local_delta & 0xff);
			return ng_local_delta_1;
		}
		case 0x49: {
			uint8_t ng_local_delta_2 = (ng_local_delta >> 8) & 0xff;
			return ng_local_delta_2;
		}
		case 0x4a: {
			uint8_t ng_local_delta_3 = (ng_local_delta >> 16) & 0xff;
			return ng_local_delta_3;
		}
		case 0x4b: {
			uint8_t ng_local_delta_4 = (ng_local_delta >> 24) & 0xff;
			return ng_local_delta_4;
		}
		// Local 16 Alfa
		case 0x50: {
			uint16_t ng_local_16_alfa_1 = (ng_local_alfa & 0xffff);
			return ng_local_16_alfa_1;
		}
		case 0x51: {
			uint16_t ng_local_16_alfa_2 = (ng_local_alfa & 0xffff0000);
			return ng_local_16_alfa_2;
		}
		// Local 16 Beta
		case 0x52: {
			uint16_t ng_local_16_beta_1 = (ng_local_beta & 0xffff);
			return ng_local_16_beta_1;
		}
		case 0x53: {
			uint16_t ng_local_16_beta_2 = (ng_local_beta & 0xffff0000);
			return ng_local_16_beta_2;
		}
		// Local 16 Delta
		case 0x54: {
			uint16_t ng_local_16_delta_1 = (ng_local_delta & 0xffff);
			return ng_local_16_delta_1;
		}
		case 0x55: {
			uint16_t ng_local_16_delta_2 = (ng_local_delta & 0xffff0000);
			return ng_local_16_delta_2;
		}

		// Local 32 Alfa
		case 0x70: {
			return ng_local_alfa;
		}
		// Local 32 Beta
		case 0x71: {
			return ng_local_beta;
		}
		// Local 32 Delta
		case 0x72: {
			return ng_local_delta;
		}
		// Local 32 Timer
		case 0x73: {
			return ng_local_timer;
		}
		default: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetVariable: variable type %u unimplemented!", variable);
			return 0;
		}
	}
}

int32_t NGNumericGetSavegameValue(uint32_t variable) {
	switch (variable) {
		case 0x00: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: TRNG Index: Index of last item found with testposition or condition unimplemented");
			break;
		}
		case 0x01: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: TRNG Index: Index of last item performing last AnimCommand unimplemented");
			break;
		}
		case 0x02: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: TRNG Index: Item index for selected item unimplemented");
			break;
		}
		case 0x03: {
			return lara.item_number;
			break;
		}
		case 0x04: {
			NGLog(NG_LOG_TYPE_POSSIBLE_INACCURACY, "NGNumericGetSavegameValue: Lara Hands: Attached Lara Status untested!");
			return lara.gun_status;
			break;
		}
		case 0x05: {
			NGLog(NG_LOG_TYPE_POSSIBLE_INACCURACY, "NGNumericGetSavegameValue: Lara Hands: Item in the hands (current) untested!");
			return lara.gun_type;
			break;
		}
		case 0x06: {
			NGLog(NG_LOG_TYPE_POSSIBLE_INACCURACY, "NGNumericGetSavegameValue: Lara Hands: Item in the hands (following) untested!");
			return lara.request_gun_type;
			break;
		}
		case 0x07: {
			NGLog(NG_LOG_TYPE_POSSIBLE_INACCURACY, "NGNumericGetSavegameValue: Lara Hands: current weapon (not necessarily in the hand) untested!");
			return lara.last_gun_type;
			break;
		}
		case 0x08: {
			NGLog(NG_LOG_TYPE_POSSIBLE_INACCURACY, "NGNumericGetSavegameValue: Lara: Environment where lara is is untested!");
			return lara.water_status;
			break;
		}
		case 0x09: {
			NGLog(NG_LOG_TYPE_POSSIBLE_INACCURACY, "NGNumericGetSavegameValue: Lara: Climb sector test is untested!");
			return lara.climb_status;
			break;
		}
		case 0x0A: {
			return lara.air;
			break;
		}
		case 0x0B: {
			return lara.death_count;
			break;
		}
		case 0x0C: {
			return lara.flare_age;
			break;
		}
		case 0x0D: {
			return lara.weapon_item;
			break;
		}
		case 0x0E: {
			return lara.back_gun;
			break;
		}
		case 0x0F: {
			return lara.poisoned;
			break;
		}
		case 0x10: {
			return lara.dpoisoned;
			break;
		}
		case 0x11: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: Lara: special status 1 unimplemented!");
			return 0;
			break;
		}
		case 0x12: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: Lara: special status 2 unimplemented!");
			return 0;
			break;
		}
		case 0x13: {
			size_t address = (size_t)lara.target - (size_t)items;
			return 0;
			break;
		}
		case 0x14: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: Lara: Torch status in Lara's hand unimplemented!");
			return 0;
			break;
		}
		case 0x15: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: Lara: Flare unimplemented!");
			return 0;
			break;
		}
		case 0x16: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: Auto-aiming unimplemented!");
			return 0;
			break;
		}
		case 0x17: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: Horizontal rope position unimplemented!");
			return 0;
			break;
		}
		case 0x18: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: Lara on rope is unimplemented!");
			return 0;
			break;
		}
		case 0x19: {
			return lara.pistols_type_carried;
			break;
		}
		case 0x1A: {
			return lara.uzis_type_carried;
			break;
		}
		case 0x1B: {
			return lara.shotgun_type_carried;
			break;
		}
		case 0x1C: {
			return lara.crossbow_type_carried;
			break;
		}
		case 0x1D: {
			return lara.grenade_type_carried;
			break;
		}
		case 0x1E: {
			return lara.sixshooter_type_carried;
			break;
		}
		case 0x1F: {
			return lara.lasersight;
			break;
		}
		case 0x20: {
			return lara.binoculars;
			break;
		}
		case 0x21: {
			return lara.crowbar;
			break;
		}
		case 0x22: {
			return lara.mechanical_scarab;
			break;
		}
		case 0x23: {
			return lara.small_water_skin;
			break;
		}
		case 0x24: {
			return lara.big_water_skin;
			break;
		}
		case 0x25: {
			return lara.examine1;
			break;
		}
		case 0x26: {
			return lara.examine2;
			break;
		}
		case 0x27: {
			return lara.examine3;
			break;
		}
		case 0x28: {
			return lara.puzzleitems[0];
			break;
		}
		case 0x29: {
			return lara.puzzleitems[1];
			break;
		}
		case 0x2A: {
			return lara.puzzleitems[2];
			break;
		}
		case 0x2B: {
			return lara.puzzleitems[3];
			break;
		}
		case 0x2C: {
			return lara.puzzleitems[4];
			break;
		}
		case 0x2D: {
			return lara.puzzleitems[5];
			break;
		}
		case 0x2E: {
			return lara.puzzleitems[6];
			break;
		}
		case 0x2F: {
			return lara.puzzleitems[7];
			break;
		}
		case 0x30: {
			return lara.puzzleitems[8];
			break;
		}
		case 0x31: {
			return lara.puzzleitems[9];
			break;
		}
		case 0x32: {
			return lara.puzzleitems[10];
			break;
		}
		case 0x33: {
			return lara.puzzleitems[11];
			break;
		}
		case 0x34: {
			return lara.puzzleitemscombo & 0xff;
		}
		case 0x35: {
			return (lara.puzzleitemscombo & 0xff00) >> 8;
		}
		case 0x36: {
			return lara.keyitems & 0xff;
		}
		case 0x37: {
			return (lara.keyitems & 0xff00) >> 8;
		}
		case 0x38: {
			return (lara.pickupitems & 0xff);
		}
		case 0x39: {
			return (lara.questitems & 0xff);
		}
		case 0x3A: {
			return lara.num_small_medipack;
		}
		case 0x3B: {
			return lara.num_large_medipack;
		}
		case 0x3C: {
			return lara.num_flares;
		}
		case 0x3D: {
			return lara.num_pistols_ammo;
		}
		case 0x3E: {
			return lara.num_uzi_ammo;
		}
		case 0x3F: {
			return lara.num_revolver_ammo;
		}
		case 0x40: {
			return lara.num_shotgun_ammo1;
		}
		case 0x41: {
			return lara.num_shotgun_ammo2;
		}
		case 0x42: {
			return lara.num_grenade_ammo1;
		}
		case 0x43: {
			return lara.num_grenade_ammo2;
		}
		case 0x44: {
			return lara.num_grenade_ammo3;
		}
		case 0x45: {
			return lara.num_crossbow_ammo1;
		}
		case 0x46: {
			return lara.num_crossbow_ammo2;
		}
		case 0x47: {
			return lara.num_crossbow_ammo3;
		}
		case 0x48: {
			return lara.beetle_uses;
		}
		case 0x49: {
			return savegame.CurrentLevel;
		}
		case 0x4A: {
			return savegame.Game.Timer;
		}
		case 0x4B: {
			return savegame.Game.Distance;
		}
		case 0x4C: {
			return savegame.Game.AmmoUsed;
		}
		case 0x4D: {
			return savegame.Game.Secrets;
		}
		case 0x4E: {
			return savegame.Game.HealthUsed;
		}
		case 0x4F: {
			return savegame.Level.Timer;
		}
		case 0x50: {
			return savegame.Level.Kills;
		}
		default: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericGetSavegameValue: Unimplemented savegame value: %u", variable);
			break;
		}
	}

	return -1;
}

void NGNumericSetSavegameValue(uint32_t variable, int32_t value) {
	switch (variable) {
		case 0x00: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: TRNG Index: Index of last item found with testposition or condition unimplemented");
			break;
		}
		case 0x01: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: TRNG Index: Index of last item performing last AnimCommand unimplemented");
			break;
		}
		case 0x02: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: TRNG Index: Item index for selected item unimplemented");
			break;
		}
		case 0x03: {
			lara.item_number = (int16_t)(value & 0xffff);
			break;
		}
		case 0x04: {
			lara.gun_status = (int16_t)(value & 0xffff);
			break;
		}
		case 0x05: {
			lara.gun_type = (int16_t)(value & 0xffff);
			break;
		}
		case 0x06: {
			lara.request_gun_type = (int16_t)(value & 0xffff);
			break;
		}
		case 0x07: {
			lara.last_gun_type = (int16_t)(value & 0xffff);
			break;
		}
		case 0x08: {
			lara.water_status = (int16_t)(value & 0xffff);
			break;
		}
		case 0x09: {
			lara.climb_status = (int16_t)(value & 0xffff);
			break;
		}
		case 0x0a: {
			lara.air = (int16_t)(value & 0xffff);
			break;
		}
		case 0x0b: {
			lara.death_count = (int16_t)(value & 0xffff);
			break;
		}
		case 0x0c: {
			lara.flare_age = (int16_t)(value & 0xffff);
			break;
		}
		case 0x0d: {
			lara.weapon_item = (int16_t)(value & 0xffff);
			break;
		}
		case 0x0e: {
			lara.back_gun = (int16_t)(value & 0xffff);
			break;
		}
		case 0x0f: {
			lara.poisoned = (int16_t)(value & 0xffff);
			break;
		}
		case 0x10: {
			lara.dpoisoned = (int16_t)(value & 0xffff);
			break;
		}
		case 0x11: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara: special status 1 unimplemented!");
			return;
			break;
		}
		case 0x12: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara: special status 2 unimplemented!");
			return;
			break;
		}
		case 0x13: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara: item target memory address unimplemented!");
			return;
			break;
		}
		case 0x14: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara: Torch status in Lara's hand unimplemented!");
			return;
			break;
		}
		case 0x15: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara: Flare unimplemented!");
			return;
			break;
		}
		case 0x16: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Auto-aiming unimplemented!");
			return;
			break;
		}
		case 0x17: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Horizontal rope position unimplemented!");
			return;
			break;
		}
		case 0x18: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara on rope is unimplemented!");
			return;
			break;
		}
		case 0x19: {
			lara.pistols_type_carried = (int8_t)(value & 0xff);
			break;
		}
		case 0x1A: {
			lara.uzis_type_carried = (int8_t)(value & 0xff);
			break;
		}
		case 0x1B: {
			lara.shotgun_type_carried = (int8_t)(value & 0xff);
			break;
		}
		case 0x1C: {
			lara.crossbow_type_carried = (int8_t)(value & 0xff);
			break;
		}
		case 0x1D: {
			lara.grenade_type_carried = (int8_t)(value & 0xff);
			break;
		}
		case 0x1E: {
			lara.sixshooter_type_carried = (int8_t)(value & 0xff);
			break;
		}
		case 0x1F: {
			lara.lasersight = (int8_t)(value & 0xff);
			break;
		}
		case 0x20: {
			lara.binoculars = (int8_t)(value & 0xff);
			break;
		}
		case 0x21: {
			lara.crowbar = (int8_t)(value & 0xff);
			break;
		}
		case 0x22: {
			lara.mechanical_scarab = (int8_t)(value & 0xff);
			break;
		}
		case 0x23: {
			lara.small_water_skin = (int8_t)(value & 0xff);
			break;
		}
		case 0x24: {
			lara.big_water_skin = (int8_t)(value & 0xff);
			break;
		}
		case 0x25: {
			lara.examine1 = (int8_t)(value & 0xff);
			break;
		}
		case 0x26: {
			lara.examine2 = (int8_t)(value & 0xff);
			break;
		}
		case 0x27: {
			lara.examine3 = (int8_t)(value & 0xff);
			break;
		}
		case 0x28: {
			lara.puzzleitems[0] = (int8_t)(value & 0xff);
			break;
		}
		case 0x29: {
			lara.puzzleitems[1] = (int8_t)(value & 0xff);
			break;
		}
		case 0x2A: {
			lara.puzzleitems[2] = (int8_t)(value & 0xff);
			break;
		}
		case 0x2B: {
			lara.puzzleitems[3] = (int8_t)(value & 0xff);
			break;
		}
		case 0x2C: {
			lara.puzzleitems[4] = (int8_t)(value & 0xff);
			break;
		}
		case 0x2D: {
			lara.puzzleitems[5] = (int8_t)(value & 0xff);
			break;
		}
		case 0x2E: {
			lara.puzzleitems[6] = (int8_t)(value & 0xff);
			break;
		}
		case 0x2F: {
			lara.puzzleitems[7] = (int8_t)(value & 0xff);
			break;
		}
		case 0x30: {
			lara.puzzleitems[8] = (int8_t)(value & 0xff);
			break;
		}
		case 0x31: {
			lara.puzzleitems[9] = (int8_t)(value & 0xff);
			break;
		}
		case 0x32: {
			lara.puzzleitems[10] = (int8_t)(value & 0xff);
			break;
		}
		case 0x33: {
			lara.puzzleitems[11] = (int8_t)(value & 0xff);
			break;
		}
		case 0x34: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara: Combo items  1 - 4 unimplemented!");
			break;
		}
		case 0x35: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara: Combo items  5 - 8 unimplemented!");
			break;
		}
		case 0x36: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara: Keys  1 - 4 unimplemented!");
			break;
		}
		case 0x37: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Lara: Keys  5 - 8 unimplemented!");
			break;
		}
		case 0x38: {
			lara.pickupitems = (int8_t)(value & 0xff);
			break;
		}
		case 0x39: {
			lara.questitems = (int8_t)(value & 0xff);
			break;
		}
		case 0x3A: {
			lara.num_small_medipack = (int16_t)(value & 0xffff);
			break;
		}
		case 0x3B: {
			lara.num_large_medipack = (int16_t)(value & 0xffff);
			break;
		}
		case 0x3C: {
			lara.num_flares = (int16_t)(value & 0xffff);
			break;
		}
		case 0x3D: {
			lara.num_pistols_ammo = (int16_t)(value & 0xffff);
			break;
		}
		case 0x3E: {
			lara.num_uzi_ammo = (int16_t)(value & 0xffff);
			break;
		}
		case 0x3F: {
			lara.num_revolver_ammo = (int16_t)(value & 0xffff);
			break;
		}
		case 0x40: {
			lara.num_shotgun_ammo1 = (int16_t)(value & 0xffff);
			break;
		}
		case 0x41: {
			lara.num_shotgun_ammo2 = (int16_t)(value & 0xffff);
			break;
		}
		case 0x42: {
			lara.num_grenade_ammo1 = (int16_t)(value & 0xffff);
			break;
		}
		case 0x43: {
			lara.num_grenade_ammo2 = (int16_t)(value & 0xffff);
			break;
		}
		case 0x44: {
			lara.num_grenade_ammo3 = (int16_t)(value & 0xffff);
			break;
		}
		case 0x45: {
			lara.num_crossbow_ammo1 = (int16_t)(value & 0xffff);
			break;
		}
		case 0x46: {
			lara.num_crossbow_ammo2 = (int16_t)(value & 0xffff);
			break;
		}
		case 0x47: {
			lara.num_crossbow_ammo3 = (int16_t)(value & 0xffff);
			break;
		}
		case 0x48: {
			lara.beetle_uses = (int8_t)(value & 0xff);
			break;
		}
		case 0x49: {
			savegame.CurrentLevel = (int8_t)(value & 0xff);
			break;
		}
		case 0x4A: {
			savegame.Game.Timer = (int32_t)(value & 0xffffffff);
			break;
		}
		case 0x4B: {
			savegame.Game.Distance = (int32_t)(value & 0xffffffff);
			break;
		}
		case 0x4C: {
			savegame.Game.AmmoUsed = (int16_t)(value & 0xffff);
			break;
		}
		case 0x4D: {
			savegame.Game.Secrets = (int8_t)(value & 0xff);
			break;
		}
		case 0x4E: {
			savegame.Game.HealthUsed = (int8_t)(value & 0xff);
			break;
		}
		case 0x4F: {
			savegame.Level.Timer = (int32_t)(value & 0xffffffff);
			break;
		}
		case 0x50: {
			savegame.Level.Kills = (int16_t)(value & 0xffff);
			break;
		}
		default: {
			NGLog(NG_LOG_TYPE_UNIMPLEMENTED_FEATURE, "NGNumericSetSavegameValue: Unimplemented savegame value: %u", variable);
			break;
		}
	}
}