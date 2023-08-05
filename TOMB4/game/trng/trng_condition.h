enum NG_CONDITION_TYPE {
	INVENTORY_ITEM_IS_PRESENT = 2,
	INVENTORY_ITEM_HAS_AT_LEAST = 3,
	KEYBOARD_SCANCODE_IS_CURRENTLY = 12,
	CREATURE_IS_CURRENTLY_OF_STATE = 14,
	CREATURE_CURRENT_ANIMATION_0_31_IS = 21,
	LARA_STATUS_IS_ENABLED_OR_DISABLED = 25,
	LARA_IS_TOUCHING_MOVEABLE = 26,
	LARA_IS_PERFORMING_ANIMATION = 30,
	LARA_IS_TOUCHING_STATIC_ITEM = 34,
	LARA_IS_HOLDING_OR_DRIVING_ITEMS = 35,
	NUMERIC_VALUE_IS_EQUAL_OR_GREATER_TO = 41,
	NUMERIC_VALUE_IS_LESS_THAN = 42,
	NUMERIC_VALUE_IS_EQUAL_TO = 43,
	LARA_IS_LESS_OR_EVEN_CLICKS_DISTANT_TO_MOVEABLE = 54,
	LARA_IS_TOUCHING_MOVEABLE_WITH_MESH_NUMBER = 83,
};


extern bool NGCondition(short param, unsigned char extra, short timer);