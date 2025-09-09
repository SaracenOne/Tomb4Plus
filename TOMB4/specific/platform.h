#include <string>
#include <stdio.h>

extern std::string working_dir_path;

extern std::string game_user_dir_path;
extern std::string savegame_dir_path;
extern std::string screenshots_dir_path;

#ifdef _WIN32
#include <direct.h>
#define PATH_SEPARATOR "\\"
#else
#include <sys/stat.h>
#define PATH_SEPARATOR "/"
#endif

int32_t platform_strcicmp(char const* a, char const* b);
FILE *platform_fopen(const char* filename, const char* mode);
void platform_find_file_with_substring(const char* dir_path, const char* substring, char* found_filename);
int32_t platform_string_ends_with(const char* str, const char* suffix);
void platform_fatal_error(const char* s, ...);
void platform_message_box(const char* s, ...);
bool platform_create_directory(const char* path);
std::string platform_get_userdata_path();