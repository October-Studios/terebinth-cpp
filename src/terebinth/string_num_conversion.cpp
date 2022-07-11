#include "string_num_conversion.h"
#include <cstring>

namespace str {

std::string CharToCppStringLiteralEscaped(unsigned char c) {
  std::string out = "\\";
	
	for (int i = 0; i < 3; ++i)	{
		out += c % 8 + '0';
		c /= 8;
	}
	
	return out;
}

std::string IntToBase62(unsigned int num, int max_digits) {
	int i = 0;
  std::string out;
	
	while (num != 0 && (max_digits < 0 || i < max_digits)) {
		unsigned int digit = num % 62;
		
		if (digit < 10)	{
			out += (char)(digit + '0');
		} else if (digit < 10 + 26)	{
			out += (char)(digit - 10 + 'a');
		}	else if (digit < 10 + 26 + 26) {
			out += (char)(digit - 10 - 26 + 'A');
		}	else {
			out += "_";
		}
		
		num /= 62;
		i++;
	}
	
	return out;
}

std::string PtrToUniqueStr(void* ptr_in, int digits)
{
	unsigned long long data = 0;
	memcpy(&data, &ptr_in, sizeof(void*));
	return IntToBase62((int)data, digits);
}

} // namespace str
