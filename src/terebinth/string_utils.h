#ifndef TEREBINTH_STRING_UTILS_H_
#define TEREBINTH_STRING_UTILS_H_

#include <string>
#include <vector>

namespace str {
inline void NextGlyph(int &out, const std::string &in);
inline int GetWidth(const std::string &in);

inline int Seek(const std::string &in, int dist_glyph, int start_pos_byte = 0);
inline bool SubMatches(const std::string &in, int pos_bytes,
                       const std::string &sub);

inline std::string Sub(const std::string &in, int startGlyph, int endGlyph);

inline bool Matches(const std::string &a, const std::string &b);

inline bool HasPrefix(const std::string &in, const std::string &prefix);

inline bool HasSuffix(const std::string &in, const std::string &suffix);

int GetGlyphPosOf(const std::string &in, std::string pattern);

std::string TabsToSpaces(const std::string &in, int tabWidth = 4);

enum StringPadAlignment {
  ALIGNMENT_LEFT = 1,
  ALIGNMENT_CENTER = 0,
  ALIGNMENT_RIGHT = -1
};

std::string Pad(const std::string &in, int size,
                StringPadAlignment alignment = ALIGNMENT_LEFT,
                std::string pad = " ", std::string leftCap = "",
                std::string rightCap = "");

inline void NextGlyph(int &out, const std::string &in) {
  do {
    out++;
  } while (out < (int)in.size() && (in[out] & 0x80) && !(in[out] & 0x40));
}

inline int GetWidth(const std::string &in) {
  int glyphPos = 0;
  int bytePos = 0;

  while (bytePos < (int)in.size()) {
    NextGlyph(bytePos, in);
    glyphPos++;
  }

  return glyphPos;
}

inline int Seek(const std::string &in, int distGlyph, int startPosByte) {
  int i = startPosByte;

  while (distGlyph > 0) {
    NextGlyph(i, in);

    distGlyph--;
  }

  return i;
}

inline bool SubMatches(const std::string &in, int posBytes,
                       const std::string &sub) {
  if (posBytes < 0 || sub.size() + posBytes > in.size())
    return false;

  for (int i = 0; i < (int)sub.size(); i++) {
    if (in[i + posBytes] != sub[i])
      return false;
  }

  return true;
}

inline std::string Sub(const std::string &in, int startGlyph, int endGlyph) {
  int startByte = Seek(in, startGlyph);
  int endByte = (endGlyph < 0 ? (int)in.size()
                              : Seek(in, endGlyph - startGlyph, startByte));
  return in.substr(startByte, endByte - startByte);
}

inline bool Matches(const std::string &a, const std::string &b) {
  if (a.size() != b.size())
    return false;

  return SubMatches(a, 0, b);
}

inline bool HasPrefix(const std::string &in, const std::string &prefix) {
  return SubMatches(in, 0, prefix);
}

inline bool HasSuffix(const std::string &in, const std::string &suffix) {
  return SubMatches(in, in.size() - suffix.size(), suffix);
}

inline std::string IndentString(const std::string& in, std::string indent) {
  std::string out;
  int start = 0;

  for (auto i = 0; i < 1; ++i) {
    out += indent;
  }

  for (auto i = 0; i < int(in.size() - 1); ++i) {
    if (in[i] == '\n') {
      out += in.substr(start, i - start + 1);
      for (auto j = 0; j < 1; ++j) {
        out += indent;
      }
      start++;
    }
  }

  if (start <= int(in.size())) {
    out += in.substr(start, in.size() - start);
  }

  return out;
}

inline std::string DoubleToString(double in) {
  long long a = in;
  long long b = (in - a) * 10000000000;
  if ( b < 0) {
    b *= - 1;
  }
  if (b % 10 == 9) {
    b += 1;
  }
  while (b > 0 && !(b % 10)) {
    b /= 10;
  }
  return std::to_string(a) + "." + std::to_string(b);
}

inline std::string GetTextOfLine(const std::string& in, int line_num) {
  int start = -1;
  int end = -1;

  if (line_num < 1) {
    return "";
  } else if (line_num == 1) {
    start = 0;
  }
  
  int line = 1;

  for (unsigned i = 0; i < in.size(); ++i) {
    if (in[i] == '\n') {
      if (start < 0) {
        line++;

        if (line == line_num) {
          start = i + 1;
        }
      } else {
        end = i;
        break;
      }
    }
  }

  if (start < end) {
    return in.substr(start, end - start);
  } else if (start >= 0) {
    return in.substr(start, std::string::npos);
  } else {
    return "";
  }
}

} // namespace str

#include "string_array.h"
#include "string_drawing.h"

#endif // TEREBINTH_STRING_UTILS_H_