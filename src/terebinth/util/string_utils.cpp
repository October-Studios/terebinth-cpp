module util.string_utils;

import <math.h>;

namespace str {

void SplitBy(std::vector<std::string> &out, const std::string &in,
             const std::string &splitter, bool keep_splitter) {
  int i = 0;
  int start = 0;

  while (i <= (int)in.size() - (int)splitter.size()) {
    if (SubMatches(in, i, splitter)) {
      if (keep_splitter)
        NextGlyph(i, in);

      out.push_back(in.substr(start, i - start));

      if (!keep_splitter)
        NextGlyph(i, in);

      start = i;
    } else {
      NextGlyph(i, in);
    }
  }

  if ((int)in.size() != start)
    out.push_back(in.substr(start, in.size() - start));
}

std::string Pad(const std::string &in, int size, StringPadAlignment alignment,
                std::string pad, std::string left_cap, std::string right_cap) {
  int cap_width = GetWidth(left_cap) + GetWidth(right_cap);
  int in_size = GetWidth(in);
  int pad_size = size - (in_size + cap_width);

  if (pad_size < 0) // need to chop
  {
    if (size - cap_width >= 1) {
      if (alignment == ALIGNMENT_RIGHT) {
        return left_cap + "…" +
               Sub(in, in_size - (size - cap_width - 1), in_size) + right_cap;
      } else {
        return left_cap + Sub(in, 0, size - cap_width - 1) + "…" + right_cap;
      }
    } else {
      return Sub(left_cap + right_cap, 0, size);
    }
  } else if (pad_size == 0) {
    return left_cap + in + right_cap;
  } else {
    if (alignment == ALIGNMENT_CENTER) {
      std::string left_pad, right_pad;

      for (auto i = 0; i < floor(pad_size / 2.0); i++)
        left_pad += pad;

      for (auto i = 0; i < ceil(pad_size / 2.0); i++)
        right_pad += pad;

      return left_pad + left_cap + in + right_cap + right_pad;
    }
    // left or right alignment
    else {
      std::string pad_str;

      for (int i = 0; i < pad_size; i++)
        pad_str += pad;

      if (alignment == ALIGNMENT_RIGHT) // right align
        return pad_str + left_cap + in + right_cap;
      else // left align
        return left_cap + in + right_cap + pad_str;
    }
  }
}

std::string TabsToSpaces(const std::string &in, int tab_width) {
  int width_since_newline = 0;
  std::string out;
  int byte_pos = 0;
  int start = 0;

  while (byte_pos < (int)in.size()) {
    if (SubMatches(in, byte_pos, "\t")) {
      out += in.substr(start, byte_pos - start);
      std::string spacer;
      int spaces = tab_width - (width_since_newline % tab_width);
      for (int i = 0; i < spaces; i++)
        spacer += " ";
      out += spacer;
      width_since_newline += spaces;
      NextGlyph(byte_pos, in);
      start = byte_pos;
    } else {
      width_since_newline++;
      if (SubMatches(in, byte_pos, "\n")) {
        width_since_newline = 0;
      }
      NextGlyph(byte_pos, in);
    }
  }

  out += in.substr(start, byte_pos - start);

  return out;
}

int GetGlyphPosOf(const std::string &in, std::string pattern) {
  int glyph = 0;
  int byte = 0;

  while (byte < (int)in.size()) {
    if (SubMatches(in, byte, pattern))
      return glyph;

    NextGlyph(byte, in);
    glyph++;
  }

  return -1;
}

} // namespace str
