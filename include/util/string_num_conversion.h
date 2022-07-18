#ifndef TEREBINTH_STRING_NUM_CONVERSION_H_
#define TEREBINTH_STRING_NUM_CONVERSION_H_

#include "string_utils.h"

namespace str {
std::string CharToCppStringLiteralEscaped(unsigned char c);

std::string IntToBase62(unsigned int in, int max_digits = -1);

std::string PtrToUniqueStr(void *ptr_in, int digits = 4);
} // namespace str

#endif // TEREBINTH_STRING_NUM_CONVERSION_H_