#include "../../../pch.h"
#include "t4plus_plugin_akyvmix_01.h"

void T4PlusPluginInitProgramLog_AkyVMix01(int32_t log_type, const char* string, ...) {
	PLUGIN_LOG(PLUGIN_NAME, log_type, string)
};

void T4PlusPluginInitProgram_AkyVMix01() {
	T4PlusPluginInitProgramLog_AkyVMix01(0, "Initializing");
}

void T4PlusPluginConditionCustom_AkyVMix01(int16_t condition_index, int32_t item_index, int16_t extra, T4PlusActivationMode activation_mode) {
	T4PlusPluginInitProgramLog_AkyVMix01(0, "Conditional %i is unimplemented", condition_index);
}

void T4PlusPluginActionCustom_AkyVMix01(int16_t action_index, int32_t item_index, int16_t extra, T4PlusActivationMode activation_mode) {
	T4PlusPluginInitProgramLog_AkyVMix01(0, "Action %i is unimplemented", action_index);
}

void T4PlusPluginFlipEffectCustom_AkyVMix01(int16_t flip_index, int16_t timer, int16_t extra, T4PlusActivationMode activation_mode) {
	T4PlusPluginInitProgramLog_AkyVMix01(0, "Flipeffect %i is unimplemented", flip_index);
}

void T4PlusPluginRegister_AkyVMix01() {

}

T4PlusPluginInterface AkyVMix01PluginStructure;

T4PlusPluginInterface* T4PlusPluginGetPluginStructure_AkyVMix01() {
	AkyVMix01PluginStructure.register_plugin = T4PlusPluginRegister_AkyVMix01;

	return &AkyVMix01PluginStructure;
}