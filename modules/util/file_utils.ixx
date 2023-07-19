module;
export module util.file_utils;
import util.string_utils;

export {
void LoadFile(std::string filepath, std::string &contents);
void WriteFile(std::string filepath, const std::string &contents);
}  // export
