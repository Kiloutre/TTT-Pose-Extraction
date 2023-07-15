#include <filesystem>

#include "Utils.hpp"

bool fileExists(const char* name) {
	struct stat buffer;
	return (stat(name, &buffer) == 0);
}

