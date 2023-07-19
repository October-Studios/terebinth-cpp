module;
#include <math.h>
#ifdef __linux__
#include <sys/ioctl.h>
#include <unistd.h>
#else
#include <io.h>
#define popen _popen
#define pclose _pclose
#endif

#include <exception>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
export module util.string_utils;

import error_handler;

export {

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
                std::string pad = " ", std::string left_cap = "",
                std::string right_cap = "");

inline std::string PadString(const std::string &in, int size, int alignment = 1,
                             std::string pad = " ", std::string left_cap = "",
                             std::string right_cap = "") {
  int cap_size = left_cap.size() + right_cap.size();
  int pad_size = size - int(in.size() + cap_size);

  if (pad_size < 0) {
    if (size - cap_size >= 1) {
      return left_cap + in.substr(0, size - cap_size - 1) + "…" + right_cap;
    } else if (size - cap_size >= 0) {
      return left_cap + std::string("…").substr(0, size - cap_size) + right_cap;
    } else {
      return (left_cap + right_cap).substr(0, size);
    }
  } else if (pad_size == 0) {
    return left_cap + in + right_cap;
  } else {
    if (alignment == 0) {
      std::string left_pad, right_pad;

      for (int i = 0; i < floor(pad_size / 2.0); i++) left_pad += pad;

      for (int i = 0; i < ceil(pad_size / 2.0); i++) right_pad += pad;

      return left_pad + left_cap + in + right_cap + right_pad;
    }
    // left or right alignment
    else {
      std::string pad_str;

      for (int i = 0; i < pad_size; i++) pad_str += pad;

      if (alignment > 0)  // right align
        return left_cap + in + right_cap + pad_str;
      else  // left align
        return pad_str + left_cap + in + right_cap;
    }
  }
}

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
  if (posBytes < 0 || sub.size() + posBytes > in.size()) return false;

  for (int i = 0; i < (int)sub.size(); i++) {
    if (in[i + posBytes] != sub[i]) return false;
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
  if (a.size() != b.size()) return false;

  return SubMatches(a, 0, b);
}

inline bool HasPrefix(const std::string &in, const std::string &prefix) {
  return SubMatches(in, 0, prefix);
}

inline bool HasSuffix(const std::string &in, const std::string &suffix) {
  return SubMatches(in, in.size() - suffix.size(), suffix);
}

inline std::string IndentString(const std::string &in, std::string indent) {
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
  if (b < 0) {
    b *= -1;
  }
  if (b % 10 == 9) {
    b += 1;
  }
  while (b > 0 && !(b % 10)) {
    b /= 10;
  }
  return std::to_string(a) + "." + std::to_string(b);
}

inline double StringToDouble(std::string in) {
  double out = 0;
  int divider = 1;

  for (auto i = 0; i < (int)in.size(); ++i) {
    if (divider == 1) {
      if (in[i] >= '0' && in[i] <= '9') {
        out = out * 10 + in[i] - '0';
      } else if (in[i] == '.') {
        divider = 10;
      }
    } else {
      if (in[i] >= '0' && in[i] <= '9') {
        out += (double)(in[i] - '0') / divider;
        divider *= 10;
      }
    }
  }

  if (in.size() > 0 && in[0] == '-') {
    out *= -1;
  }

  return out;
}

inline int StringToInt(std::string in) {
  int out = 0;

  for (auto i = 0; i < (int)in.size(); ++i) {
    if (in[i] >= '0' && in[i] <= '9') {
      out = out * 10 + in[i] - '0';
    } else if (in[i] == '.') {
      break;
    }
  }

  if (in.size() > 0 && in[0] == -1) {
    out *= -1;
  }

  return out;
}

inline std::string RunCmd(std::string cmd, bool print_output = false) {
  std::string result = "";
  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe) throw std::runtime_error("popen() failed in GetOutputFromCmd");
  try {
    while (!feof(pipe)) {
      char c;
      if ((c = getc(pipe)) != EOF) {
        result += c;
        if (print_output) {
          std::cout << c;
          std::cout.flush();
        }
      }
    }
  } catch (...) {
    pclose(pipe);
    throw;
  }
  pclose(pipe);
  return result;
}

inline std::string GetTextOfLine(const std::string &in, int line_num) {
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

inline int GetTermWidth() {
#ifdef __linux__
  static bool first_time = true;
  if (first_time) {
    usleep(20000);
    first_time = false;
  }

  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

  return w.ws_col;
#else
  return 80;
#endif
}

inline int SearchInString(const std::string &in, const std::string &pattern,
                          int start_pos) {
  for (auto i = start_pos; i < in.size(); ++i) {
    if (SubMatches(in, i, pattern)) {
      return i;
    }
  }

  return -1;
}

inline void SliceStringBy(const std::string &in, const std::string &pattern,
                          std::vector<std::string> &out) {
  int start = 0;

  if (pattern.size() < 1) {
    return;
  }

  while (start < int(in.size())) {
    int end = SearchInString(in, pattern, start);

    if (end < 0) {
      end = in.size();
    }

    out.push_back(in.substr(start, end - start));

    start = end + pattern.size();
  }
}

inline void TabsToSpaces(std::vector<std::string> &in) {
  for (auto i = 0; i < in.size(); ++i) {
    in[i] = TabsToSpaces(in[i]);
  }
}

inline std::string LineListToBoxedString(const std::vector<std::string> &in,
                                         std::string box_name, int line_num,
                                         bool always_width_max, int max_width) {
  std::string out;

  auto first = in.begin();
  auto last = in.end();

  if (first != last) {
    last--;

    while (first != last && *first == "") {
      first++;
      if (line_num >= 0) {
        line_num++;
      }
    }

    while (first != last && *last == "") {
      last--;
    }

    last++;
  }

  int extra_width = (line_num < 0) ? 4 : 10;

  int size;

  if (always_width_max) {
    size = max_width - extra_width;
  } else {
    size = box_name.size() - extra_width + 6;

    for (auto i : in) {
      size = std::max(size, int(i.size()));
    }

    size = std::min(max_width - extra_width, size);
  }

  if (box_name == "") {
    out += "  " + PadString("", size + extra_width, 1, "_") + "  ";
  } else {
    out += "  _" +
           PadString(box_name, size + extra_width - 2, 0, "_", "[ ", " ]") +
           "_  ";
  }

  out += "\n |" + PadString("", size + extra_width, 1, " ") + "| ";

  auto i = first;

  while (i != last) {
    if (line_num < 0) {
      out += "\n |  ";
    } else {
      out += "\n |" + PadString(std::to_string(line_num), 4, -1) + "   ";
      line_num++;
    }

    out += PadString(*i, size, 1) + "  | ";

    i++;
  }

  out += "\n |" + PadString("", size + extra_width, 1, "_") + "| \n";

  return out;
}

inline char GetRandChar() {
  static unsigned int seed = 1;

  seed = (seed * 1103515245U + 12345U) & 0x7fffffffU;
  int num = seed % (26 + 26 + 10);

  if (num < 26) {
    return num + 'a';
  } else if (num < 26 + 26) {
    return num - 26 + 'A';
  } else if (num < 26 + 26 + 10) {
    return num - 26 - 26 + '0';
  }

  return '_';
}

inline std::string GetUniqueString(std::string hint,
                                   std::function<bool(std::string)> checker,
                                   bool always_append_random) {
  std::string out = hint;

  int attempts = 0;
  bool invalid = out.empty() || !checker(out) || always_append_random;

  while (invalid) {
    if (always_append_random || attempts >= 10) {
      out = hint + "_";

      if (attempts > 20) {
        throw TerebinthError("could not find unique random name",
                             INTERNAL_ERROR);
      }

      for (int i = 0; i < 3; ++i) {
        out += GetRandChar();
      }
    } else {
      std::string suffix = std::to_string(attempts);

      out = hint + "_" + suffix;
    }
    attempts++;
    invalid = !checker(out);
  }

  return out;
}

inline std::string PutStringInBox(const std::string &in,
                                  std::string box_name = "",
                                  bool show_line_nums = false,
                                  bool always_width_max = false,
                                  int max_width = -1) {
  std::vector<std::string> lines;

  if (max_width < 0) {
    max_width = GetTermWidth() - 4;
  }

  SliceStringBy(in, "\n", lines);

  TabsToSpaces(lines);

  std::string out = LineListToBoxedString(
      lines, box_name, show_line_nums ? 1 : -1, always_width_max, max_width);

  return out;
}

}  // namespace str

}  // export

import util.string_array;
import util.string_drawing;