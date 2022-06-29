#ifndef TEREBINTH_TOKEN_H_
#define TEREBINTH_TOKEN_H_

#include "operator.h"

#include <memory>
#include <string>
#include <vector>

class SourceFile;

class TokenData {
public:
  enum Type {
    WHITESPACE,
    LINE_END,
    IDENTIFIER,
    LITERAL,
    STRING_LITERAL,
    OPERATOR,
    LINE_COMMENT,
    BLOCK_COMMENT,
    SCOPE,
    CAST,
    TUPLE,
    UNKNOWN
  };

  TokenData(std::string text_in, std::shared_ptr<SourceFile> file_in,
            int line_in, int char_pos_in, Type token_type_in,
            Operator op_in = Operator(nullptr)) {
    text_ = text_in;
    file_ = file_in;
    line_ = line_in;
    char_pos_ = char_pos_in;
    token_type_ = token_type_in;
    op_ = op_in;
  }

  std::string GetText() const { return text_; }

  std::shared_ptr<SourceFile> GetFile() const { return file_; }

  int GetLine() const { return line_; }

  int GetCharPos() const { return char_pos_; }

  TokenData::Type GetType() const { return token_type_; }

  Operator GetOp() const { return op_; }

  bool IsComment() {
    return token_type_ == LINE_COMMENT || token_type_ == BLOCK_COMMENT;
  }

  static std::string TypeToString(TokenData::Type in);
  std::string GetDescription();
  std::string GetTypeDescription();

private:
  std::string text_;
  std::shared_ptr<SourceFile> file_;
  int line_;
  int char_pos_;
  Type token_type_;
  Operator op_;
};

typedef std::shared_ptr<TokenData> Token;

Token MakeToken(std::string text_in, std::shared_ptr<SourceFile> file_in,
                int line_in, int char_pos_in, TokenData::Type token_type_in,
                Operator op_in = Operator(nullptr));

Token MakeToken(std::string text_in);

std::string StringFromTokens(const std::vector<Token> &tokens, int left,
                             int right);

std::string TableStringFromTokens(const std::vector<Token> &tokens,
                                  std::string table_name);

#endif // TEREBINTH_TOKEN_H_