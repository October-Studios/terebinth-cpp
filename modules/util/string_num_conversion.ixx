module;
export module util.string_num_conversion;

import util.string_utils;

export {
namespace str {
std::string CharToCppStringLiteralEscaped(unsigned char c);

std::string IntToBase62(unsigned int in, int max_digits = -1);

std::string PtrToUniqueStr(void *ptr_in, int digits = 4);
}  // namespace str

}  // export
