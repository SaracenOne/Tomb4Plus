#include "../../tomb4/pch.h"
#include "t4plus_plugin.h"

#include "builtin_plugins/AkyVMix01/t4plus_plugin_akyvmix_01.h"

int32_t t4plus_active_plugin_count = 0;
T4PlusRegisteredPlugin t4plus_registered_plugins[MAX_REGISTERED_PLUGINS];

T4PlusBuiltinPluginTableEntry t4plus_builtin_plugin_table[] = {
	{"AkyVMix01", T4PlusPluginGetPluginStructure_AkyVMix01}
};

int32_t T4PlusFindBuiltinPluginIDForBuiltinName(const char *plugin_builtin_name) {
	for (int32_t i = 0; i < (sizeof(t4plus_builtin_plugin_table) / sizeof(T4PlusBuiltinPluginTableEntry)); i++) {
		if (strcmp(plugin_builtin_name, t4plus_builtin_plugin_table[i].plugin_name) == 0) {
			return i;
		}
	}

	return -1;
}

T4PlusPluginRegistrationResult T4PlusRegisterBuiltinPlugin(const char *plugin_name, const char *plugin_builtin_name) {
	if (t4plus_active_plugin_count >= MAX_REGISTERED_PLUGINS) {
		Log(2, "Exceeded maximum registered plugins");
		return T4PLUS_PLUGIN_REGISTRATION_RESULT_FAILED;
	}

	int32_t builtin_plugin_id = T4PlusFindBuiltinPluginIDForBuiltinName(plugin_builtin_name);
	if (builtin_plugin_id >= 0) {
		t4plus_registered_plugins[t4plus_active_plugin_count].plugin_name = plugin_name;
		t4plus_registered_plugins[t4plus_active_plugin_count].plugin_type = T4PLUS_PLUGIN_TYPE_BUILTIN;
		t4plus_registered_plugins[t4plus_active_plugin_count].plugin_interface = t4plus_builtin_plugin_table[builtin_plugin_id].plugin_interface();

		t4plus_active_plugin_count++;

		return T4PLUS_PLUGIN_REGISTRATION_RESULT_OK;
	} else {
		Log(2, "Could not find builtin plugin named %s", plugin_builtin_name);
		return T4PLUS_PLUGIN_REGISTRATION_RESULT_FAILED;
	}

	return T4PLUS_PLUGIN_REGISTRATION_RESULT_FAILED;
}

int32_t T4PlusFindRegisteredPluginByName(const char* plugin_name) {
	for (int32_t i = 0; i < t4plus_active_plugin_count; i++) {
		if (strcmp(plugin_name, t4plus_registered_plugins[i].plugin_name) == 0) {
			return i;
		}
	}

	return -1;
}