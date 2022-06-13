#ifndef TEREBINTH_SOURCE_FILE_H_
#define TEREBINTH_SOURCE_FILE_H_

#include <string>

class SourceFile {
public:
  SourceFile() {
    filename = "[empty_file]";
    contents = "";
  }

  SourceFile(std::string filename_in, bool print_output);

  std::string GetFilename() { return filename; }

  // get the path to the directory this file is in
  std::string GetDirPath();

  const std::string &GetContents() { return contents; }

  std::string GetBoxedString();

  std::string GetLine(int lineNum);

  inline int Size() { return contents.size(); }

  inline char operator[](int i) { return contents[i]; }

  inline std::string Substr(size_t start, size_t len) {
    return contents.substr(start, len);
  }

private:
  std::string filename;
  std::string contents;
};

#endif // TEREBINTH_SOURCE_FILES_H_