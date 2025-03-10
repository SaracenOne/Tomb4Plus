#include "../../tomb4/pch.h"
#include "../../specific/platform.h"

#include "trepsave.h"
#include "furr.h"
#include "../../tomb4/tomb4plus/t4plus_weather.h"

#define TREPSAVE_BUFFER_SIZE 0x2ff0

void S_TREPLoadgame(int32_t slot_num) {
	size_t bytes_read = 0;

	char buffer[TREPSAVE_BUFFER_SIZE];
	memset(buffer, 0x00, TREPSAVE_BUFFER_SIZE);

	sprintf(buffer, "trepsave.%d", slot_num);

	std::string full_path = savegame_dir_path + buffer;

	FILE* file = platform_fopen(full_path.c_str(), "rb");

	furr_clear_oneshot_buffer();
	t4p_rain_type = T4P_WEATHER_DISABLED;
	t4p_snow_type = T4P_WEATHER_DISABLED;

	if (file) {
		bytes_read = fread(buffer, 1, TREPSAVE_BUFFER_SIZE, file);
		if (bytes_read == TREPSAVE_BUFFER_SIZE) {
			memcpy(furr_oneshot_buffer, buffer, LAST_FURR_FLIPEFFECT);

			int8_t weather = 0;
			memcpy(&weather, buffer + (LAST_FURR_FLIPEFFECT + 0x70), sizeof(int8_t));
			switch (weather) {
				case 1:
					t4p_rain_type = T4P_WEATHER_ENABLED_ALL_OUTSIDE;
					t4p_snow_type = T4P_WEATHER_DISABLED;
					break;
				case 2:
					t4p_rain_type = T4P_WEATHER_DISABLED;
					t4p_snow_type = T4P_WEATHER_ENABLED_ALL_OUTSIDE;
					break;
				default:
					t4p_rain_type = T4P_WEATHER_DISABLED;
					t4p_snow_type = T4P_WEATHER_DISABLED;
					break;
			}
		}
		fclose(file);

		return;
	}
}

void S_TREPSavegame(int32_t slot_num) {
	size_t bytes_written = 0;

	char buffer[TREPSAVE_BUFFER_SIZE];
	memset(buffer, 0x00, TREPSAVE_BUFFER_SIZE);

	sprintf(buffer, "trepsave.%d", slot_num);

	std::string full_path = savegame_dir_path + buffer;

	FILE* file = platform_fopen(full_path.c_str(), "wb");
	if (file) {
		memcpy(buffer, furr_oneshot_buffer, LAST_FURR_FLIPEFFECT);

		int8_t weather = 0;
		if (t4p_rain_type == T4P_WEATHER_ENABLED_ALL_OUTSIDE) {
			weather += 1;
		}
		if (t4p_snow_type == T4P_WEATHER_ENABLED_ALL_OUTSIDE) {
			weather += 2;
		}

		memcpy(buffer + 0x270, &weather, sizeof(int8_t));

		bytes_written = fwrite(buffer, 1, TREPSAVE_BUFFER_SIZE, file);

		fclose(file);
	}
}