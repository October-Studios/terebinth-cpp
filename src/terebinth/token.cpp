#include "token.h"
#include "utils/string_utils.h"

Token MakeToken(std::string text_in, std::shared_ptr<SourceFile> file_in,
                int line_in, int char_pos_in, TokenData::Type token_type_in,
                Operator op_in) {
  if (str::HasPrefix(text_in, "\"") && !str::HasSuffix(text_in, "\"")) {
    text_in += "\"";
  }

  return Token(new TokenData(text_in, file_in, line_in, char_pos_in,
                             token_type_in, op_in));
}

Token MakeToken(std::string text_in) {
  return Token(new TokenData(text_in, nullptr, 0, 0, TokenData::IDENTIFIER,
                             Operator(nullptr)));
}