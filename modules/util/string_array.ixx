export module util.string_array;

import <string>;
import <vector>;

export {
namespace str {

enum StringPadAlignment {
  ALIGNMENT_LEFT = 1,
  ALIGNMENT_CENTER = 0,
  ALIGNMENT_RIGHT = -1
};

void SplitBy(std::vector<std::string> &out, const std::string &in,
             const std::string &splitter, bool keep_splitter = false);

inline void SplitByLine(std::vector<std::string> &out, const std::string &in);

int GetMaxWidth(std::vector<std::string> &in);

void PadWidths(std::vector<std::string> &out, int size = -1,
               StringPadAlignment alignment = ALIGNMENT_LEFT,
               std::string pad = " ", std::string left_cap = "",
               std::string right_cap = "");

std::string Join(std::vector<std::string> &in, std::string joiner = "\n",
                 bool add_at_end = true);

inline void SplitByLine(std::vector<std::string> &out, const std::string &in) {
  SplitBy(out, in, "\n");
}
}  // namespace str

}  // export
