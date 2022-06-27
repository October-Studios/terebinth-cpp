#include "util/file_utils.h"

#include <fstream>
#include <sstream>

void LoadFile(std::string filepath, std::string &contents) {
  std::fstream in_file;

  in_file.open(filepath);

  if (!in_file.is_open()) {
    throw "could not open '" + filepath + "'";
  } else {
    std::stringstream str_stream;
    str_stream << in_file.rdbuf();
    contents = str_stream.str();
    in_file.close();
  }
}

void WriteFile(std::string filepath, const std::string &contents) {
  std::ofstream out_file;

  out_file.open(filepath);

  if (!out_file.is_open()) {
    throw "error writing to '" + filepath + "'";
  } else {
    out_file << contents;
    out_file.close();
  }
}
