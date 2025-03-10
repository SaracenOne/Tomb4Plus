#include "../tomb4/pch.h"
#include "platform.h"

#include <string.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__unix__) || defined(__APPLE__) || defined(_POSIX_VERSION)
#include <dirent.h>
#else
#error "Platform not supported"
#endif
#include "cmdline.h"
#include "function_stubs.h"
#include "winmain.h"

std::string userdata_path_dir = "";

std::string working_dir_path;
std::string game_user_dir_path;
std::string savegame_dir_path;
std::string screenshots_dir_path;

size_t count_matching_characters(const char* s1, const char* s2) {
	int count = 0;
	while (*s1 && *s2) {
		// If the first byte is 110xxxxx or 1110xxxx or 11110xxx, it's a multi-byte character
		size_t len = (*s1 & 0xE0) == 0xC0 ? 2 : (*s1 & 0xF0) == 0xE0 ? 3 : (*s1 & 0xF8) == 0xF0 ? 4 : 1;

		if (strncmp(s1, s2, len) == 0) {
			count++;
			s1 += len;
			s2 += len;
		} else {
			break;
		}
	}
	return count;
}

int platform_strcicmp(const char* a, const char* b) {
	for (;; a++, b++) {
		int d = tolower((uint8_t)*a) - tolower((uint8_t)*b);
		if (d != 0 || !*a)
			return d;
	}
}

FILE *platform_fopen(const char *filename, const char *mode) {
#if defined(_WIN32) && defined(UNICODE)
	int len = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
	wchar_t* wpath = (wchar_t*)SYSTEM_MALLOC(len * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, filename, -1, wpath, len);

	len = MultiByteToWideChar(CP_UTF8, 0, mode, -1, NULL, 0);
	wchar_t* wmode = (wchar_t*)SYSTEM_MALLOC(len * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, len);

	FILE* file = _wfopen(wpath, wmode);

	SYSTEM_FREE(wpath);
	SYSTEM_FREE(wmode);
#else
	FILE* file = fopen(filename, mode);
#endif

	return file;
}

void platform_find_file_with_substring(const char* dir_path, const char* substring, char* found_filename) {
#ifdef _WIN32
#ifdef UNICODE
	wchar_t wide_substring[128];
	wchar_t wide_dir_path[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, substring, -1, wide_substring, sizeof(wide_substring) / sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, dir_path, -1, wide_dir_path, sizeof(wide_substring) / sizeof(wchar_t));

	wchar_t win32_path[MAX_PATH];
	swprintf(win32_path, MAX_PATH, L"%s\\*", wide_dir_path);
	WIN32_FIND_DATAW find_file_data;
#else
	char win32_path[MAX_PATH];
	sprintf(win32_path, "%s\\*", dir_path);
	WIN32_FIND_DATAA find_file_data;
#endif

	HANDLE hFind = FindFirstFile(win32_path, &find_file_data);

	if (hFind == INVALID_HANDLE_VALUE) {
		perror("Error finding file");
		return;
	}

	do {
		if (find_file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			continue; // Skip directories
		}

#ifdef UNICODE
		char cFileName[MAX_PATH];
		WideCharToMultiByte(CP_UTF8, 0, find_file_data.cFileName, -1, cFileName, sizeof(cFileName), NULL, NULL);
#else
		char* cFileName = find_file_data.cFileName;
#endif

		size_t cmp_count = count_matching_characters(substring, cFileName);
		size_t str_len = strlen(substring);

		if (cmp_count >= str_len) {
			size_t filename_length = strlen(cFileName);

			if (filename_length < 256) {
				strcpy(found_filename, cFileName);
				found_filename[filename_length] = '\0';
				break;
			} else {
				Log(1, "Filename %s too long!", cFileName);
				return;
			}
		}
	} while (FindNextFile(hFind, &find_file_data));
	FindClose(hFind);
#elif defined(_POSIX_VERSION)
	DIR* dirp = opendir(dir_path);
	if (dirp == NULL) {
		perror("Error opening directory");
		return;
	}

	struct dirent* dp;
	while ((dp = readdir(dirp)) != NULL) {
		if (dp->d_type == DT_DIR) {
			continue; // Skip directories
		}

		size_t cmp_count = count_matching_characters(substring, dp->d_name);
		size_t str_len = strlen(substring);

		if (cmp_count >= str_len) {
			int filename_length = strlen(dp->d_name);

			if (filename_length < 256) {
				strcpy(found_filename, dp->d_name);
				found_filename[filename_length] = '\0';
				break;
			} else {
				Log(1, "Filename %s too long!", dp->d_name);
				return;
			}
		}
	}

	closedir(dirp);
#endif
}

int platform_string_ends_with(const char* str, const char* suffix) {
	if (!str || !suffix)
		return 0;
	size_t len_str = strlen(str);
	size_t len_suffix = strlen(suffix);
	if (len_suffix > len_str)
		return 0;
	return strncmp(str + len_str - len_suffix, suffix, len_suffix) == 0;
}

void platform_fatal_error(const char* s, ...) {
	va_list list;
	char buf[4096];

	va_start(list, s);
	vsprintf(buf, s, list);
	strcat(buf, "\n");
	va_end(list);

	Log(0, "Fatal Error: %s", buf);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
	                         "Tomb4Main",
	                         buf,
	                         NULL);

	exit(-1);
}

void platform_message_box(const char* s, ...) {
	va_list list;
	char buf[4096];

	va_start(list, s);
	vsprintf(buf, s, list);
	strcat(buf, "\n");
	va_end(list);

	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
	                         "Tomb4Main",
	                         buf,
	                         NULL);
}

bool platform_create_directory(const char* path) {
#ifdef UNICODE
	wchar_t tmp[MAX_PATH];
	wchar_t* p = NULL;
	size_t len;

	swprintf(tmp, sizeof(tmp) / sizeof(wchar_t), L"%S", path);
	len = wcslen(tmp);
	if (tmp[len - 1] == *PATH_SEPARATOR) {
		tmp[len - 1] = 0;
	}
	for (p = tmp + 1; *p; p++) {
		if (*p == *PATH_SEPARATOR) {
			*p = 0;
			DWORD attr = GetFileAttributes(tmp);
			if (attr == INVALID_FILE_ATTRIBUTES) {
				int res = _wmkdir(tmp);
				int er = errno;

				if (res != 0 && er != EEXIST) {
					return false;
				}
			}

			if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
				int res = _wmkdir(tmp);
				int er = errno;

				if (res != 0 && er != EEXIST) {
					return false;
				}
			}
			*p = *PATH_SEPARATOR;
		}
	}
	if (_wmkdir(tmp) != 0 && errno != EEXIST) {
		return false;
	}
#else
	char tmp[256];
	char* p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", path);
	len = strlen(tmp);
	if (tmp[len - 1] == *PATH_SEPARATOR) {
		tmp[len - 1] = 0;
	}
	for (p = tmp + 1; *p; p++) {
		if (*p == *PATH_SEPARATOR) {
			*p = 0;
#ifdef _WIN32
			DWORD attr = GetFileAttributesA(tmp);
			if (attr == INVALID_FILE_ATTRIBUTES) {
				int res = _mkdir(tmp);
				int er = errno;

				if (res != 0 && er != EEXIST) {
					return false;
				}
			}

			if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
				int res = _mkdir(tmp);
				int er = errno;

				if (res != 0 && er != EEXIST) {
					return false;
				}
			}
#else
			struct stat st;
			if (stat(tmp, &st) != 0) {
				if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST) {
					return false;
				}
			} else if (!S_ISDIR(st.st_mode)) {
				if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST) {
					return false;
				}
			}
#endif
			*p = *PATH_SEPARATOR;
		}
	}
#ifdef _WIN32
	if (_mkdir(tmp) != 0 && errno != EEXIST) {
		return false;
	}
#else
	if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST) {
		return false;
	}
#endif
#endif
	return true;
}

std::string platform_get_userdata_path() {
	if (userdata_path_dir.empty()) {
		FILE *portable_file = fopen("portable.txt", "r");
		if (portable_file) {
			fclose(portable_file);
			userdata_path_dir = ".";
			userdata_path_dir += PATH_SEPARATOR;
		} else {
			userdata_path_dir = SDL_GetPrefPath("", "Tomb4Plus");
		}
	}

	return userdata_path_dir;
}