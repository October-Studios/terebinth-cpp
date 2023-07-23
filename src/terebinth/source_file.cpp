module source_file;

import error_handler;
import util.file_utils;
import util.string_utils;

SourceFile::SourceFile(std::string filename_in, bool print_output) {
  filename_ = filename_in;
  try {
    LoadFile(filename_in, contents_);
    contents_ += "\n";
  } catch (std::string& err) {
    throw TerebinthError(err, SOURCE_ERROR);
  }
}

std::string SourceFile::GetDirPath() {
  auto i = filename_.size();

  while (i >= 0 && filename_[i] != '/') {
    i--;
  }

  return filename_.substr(0, i);
}

std::string SourceFile::GetBoxedString() {
  return str::GetBoxedString(contents_, filename_, true);
}

std::string SourceFile::GetLine(int line_num) {
  return str::GetTextOfLine(contents_, line_num);
}
