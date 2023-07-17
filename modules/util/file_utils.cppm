module;
export module Util.FileUtils;
#include "string_utils.h"

export {
void LoadFile(std::string filepath, std::string &contents);
void WriteFile(std::string filepath, const std::string &contents);
}  // export
