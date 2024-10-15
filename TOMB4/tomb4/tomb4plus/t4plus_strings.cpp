#include "../../tomb4/pch.h"

#include "t4plus_strings.h"

#include <unordered_map>

std::unordered_map<std::string, std::string> string_hash_table;
std::unordered_map<std::string, std::string> string_override_hash_table;

void T4PInsertString(const char* key, const char *value) {
}


char *T4PGetString(const char* key) {
	return nullptr;
}
