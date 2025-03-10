#pragma once

enum T4POverrideFogMode {
	T4P_FOG_DEFAULT,
	T4P_FOG_FORCE_VOLUMETRIC,
	T4P_FOG_FORCE_DISTANT
};

extern T4POverrideFogMode t4_override_fog_mode;

enum T4PWeatherType {
	T4P_WEATHER_DISABLED,
	T4P_WEATHER_ENABLED_IN_SPECIFIC_ROOMS,
	T4P_WEATHER_ENABLED_ALL_OUTSIDE
};

extern T4PWeatherType t4p_rain_type; // TRLE
extern T4PWeatherType t4p_snow_type; // TRLE

extern int32_t rain_outside; // TRLE
extern int32_t snow_outside; // TRLE

// TRLE - Weather effects
extern void InitWeatherFX();
extern void ClearWeatherFX();
extern void DoWeather();