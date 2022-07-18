#pragma once

#include <string>

class SourceFile {
 public:
  SourceFile() {
    filename_ = "[empty_file]";
    contents_ = "";
  }

  SourceFile(std::string filename_in, bool print_output);

  std::string GetFilename() { return filename_; }

  // get the path to the directory this file is in
  std::string GetDirPath();

  const std::string &GetContents() { return contents_; }

  std::string GetBoxedString();

  std::string GetLine(int lineNum);

  inline size_t Size() { return contents_.size(); }

  inline char operator[](int i) { return contents_[i]; }

  inline std::string Substr(size_t start, size_t len) {
    return contents_.substr(start, len);
  }

 private:
  std::string filename_;
  std::string contents_;
};
