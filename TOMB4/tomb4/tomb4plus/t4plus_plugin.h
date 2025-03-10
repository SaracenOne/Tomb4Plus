#pragma once

#define PLUGIN_LOG(plugin_name, t, s)	va_list list;\
char buf[4096]; \
\
va_start(list, s); \
vsprintf(buf, s, list); \
va_end(list); \
\
Log(t, "plugin_name: %s", buf); \

enum T4PlusActivationMode {
    T4_PLUS_ACTIVATION_MODE_HEAVY,
    T4_PLUS_ACTIVATION_MODE_TEMP_ONESHOT,
    T4_PLUS_ACTIVATION_MODE_BUTTON_ONESHOT,
    T4_PLUS_ACTIVATION_MODE_YET_TO_PERFORM,
    T4_PLUS_ACTIVATION_MODE_SCRIPT_TRIGGER,
    T4_PLUS_ACTIVATION_MODE_DIRECT_CALL,
    T4_PLUS_ACTIVATION_MODE_FLOOR_DATA,
    T4_PLUS_ACTIVATION_MODE_ANIM_COMMAND,
};

struct T4PlusPluginInterface {
	void (*register_plugin)();
};

struct T4PlusBuiltinPluginTableEntry {
	const char *plugin_name;
	T4PlusPluginInterface *(*plugin_interface)();
};

#define MAX_REGISTERED_PLUGINS 64
#define MAX_PLUGIN_NAME_LENGTH 256

enum T4PlusPluginType {
	T4PLUS_PLUGIN_TYPE_BUILTIN,
	T4PLUS_PLUGIN_TYPE_LUA
};

struct T4PlusRegisteredPlugin {
	const char* plugin_name;
	T4PlusPluginType plugin_type;

	T4PlusPluginInterface *plugin_interface;
};

enum T4PlusPluginRegistrationResult {
	T4PLUS_PLUGIN_REGISTRATION_RESULT_FAILED,
	T4PLUS_PLUGIN_REGISTRATION_RESULT_OK,
};

extern T4PlusPluginRegistrationResult T4PlusRegisterBuiltinPlugin(const char* plugin_name, const char* plugin_builtin_name);
extern int T4PlusFindRegisteredPluginByName(const char* plugin_name);