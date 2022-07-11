#ifndef TEREBINTH_FILE_UTILS_H_
#define TEREBINTH_FILE_UTILS_H_

#include "string_utils.h"

void LoadFile(std::string filepath, std::string &contents);
void WriteFile(std::string filepath, const std::string &contents);

#endif // TEREBINTH_FILE_UTILS_H_