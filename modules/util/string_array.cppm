#pragma once

#include "string_utils.h"

namespace str {
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
