#include "string_array.h"

namespace str {

int GetMaxWidth(std::vector<std::string>& in) {
	int out = 0;
	
	for (int i = 0; i < (int)in.size(); ++i) {
		int w = GetWidth(in[i]);
		
		if (w > out)
			out = w;
	}
	
	return out;
}

void PadWidths(std::vector<std::string>& out, int size, StringPadAlignment alignment,
    std::string pad, std::string left_cap, std::string right_cap) {
	if (size < 0) {
		size = GetMaxWidth(out);
	}
	
	for (int i = 0; i < (int)out.size(); ++i)	{
		out[i] = str::Pad(out[i], size, alignment, pad, left_cap, right_cap);
	}
}

std::string Join(std::vector<std::string>& in, std::string joiner, bool add_at_end) {
  std::string out;
	
	for (int i = 0; i < (int)in.size(); ++i) {
		out += in[i];
		
		if (add_at_end || i != (int)in.size() - 1) {
			out += joiner;
    }
	}
	
	return out;
}

} // namespace str
