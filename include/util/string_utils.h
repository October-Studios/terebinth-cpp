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

} // namespace str

#include "string_array.h"
#include "string_drawing.h"

#endif // TEREBINTH_STRING_UTILS_H_