module;
#include <unordered_map>
#include <vector>
module Lexer;

import AllOperators;
import ErrorHandler;
import Operator;
import SourceFile;
import Token;

/**
 * @brief
 *
 */
class CharacterClassifier {
 public:
  enum Type {
    WHITESPACE,
    LINE_BREAK,
    NEWLINE,
    LETTER,
    DIGIT,
    OPERATOR,
    STRING_QUOTE,
    SINGLE_LINE_COMMENT,
    MULTI_LINE_COMMENT_START,
    MULTI_LINE_COMMENT_END,
    UNKNOWN
  };

  static inline auto GetTokenType(CharacterClassifier::Type type,
                                  TokenData::Type previous_type)
      -> TokenData::Type;

  inline auto Get(std::shared_ptr<SourceFile> file, int i)
      -> CharacterClassifier::Type;

 private:
  void SetUp();

 private:
  std::unordered_map<char, CharacterClassifier::Type> hm_;
  bool has_set_up_ = false;
};

CharacterClassifier character_classifier;

void CharacterClassifier::SetUp() {
  hm_[' '] = WHITESPACE;
  hm_['\t'] = WHITESPACE;
  hm_['\r'] = WHITESPACE;
  hm_['\n'] = NEWLINE;
  hm_[';'] = LINE_BREAK;

  for (char c = 'a'; c <= 'z'; ++c) {
    hm_[c] = LETTER;
  }

  for (char c = 'A'; c <= 'Z'; ++c) {
    hm_[c] = LETTER;
  }

  hm_['_'] = LETTER;

  for (char c = '0'; c <= '9'; ++c) {
    hm_[c] = DIGIT;
  }

  hm_['#'] = SINGLE_LINE_COMMENT;

  hm_['"'] = STRING_QUOTE;

  std::unordered_map<std::string, Operator>& ops_map = ops_->GetOpsMap();

  for (auto i = ops_map.begin(); i != ops_map.end(); ++i) {
    std::string str = (*i).first;
    for (unsigned j = 0; j < str.size(); j++) {
      hm_[str[j]] = OPERATOR;
    }
  }

  has_set_up_ = true;
}

inline CharacterClassifier::Type CharacterClassifier::Get(
    std::shared_ptr<SourceFile> file, int index) {
  //	set up the first time this function is called
  if (!has_set_up_) SetUp();

  //	chack fo multi line comments in a special way, because they are multi
  // character

  switch ((*file)[index]) {
    case '/':
      if (index < int(file->Size()) - 1 && (*file)[index + 1] == '/')
        return MULTI_LINE_COMMENT_START;
      break;

    case '\\':
      if (index > 0 && (*file)[index - 1] == '\\')
        return MULTI_LINE_COMMENT_END;
      break;

    case '.':  // allow a . to be a digit character only if it is followed by a
               // digit
      if (index < int(file->Size()) - 1) {
        auto i = hm_.find((*file)[index + 1]);

        if (i != hm_.end() && i->second == DIGIT) return DIGIT;
      }
      break;
  }

  char c = (*file)[index];

  auto i = hm_.find(c);

  if (i == hm_.end())
    return UNKNOWN;
  else
    return i->second;
}

inline TokenData::Type CharacterClassifier::GetTokenType(
    CharacterClassifier::Type type, TokenData::Type previous_type) {
  if (previous_type == TokenData::LINE_COMMENT) {
    if (type == NEWLINE)
      return TokenData::WHITESPACE;
    else
      return TokenData::LINE_COMMENT;
  } else if (previous_type == TokenData::BLOCK_COMMENT) {
    if (type == MULTI_LINE_COMMENT_END)
      return TokenData::WHITESPACE;
    else
      return TokenData::BLOCK_COMMENT;
  } else if (previous_type == TokenData::STRING_LITERAL) {
    if (type == STRING_QUOTE)
      return TokenData::WHITESPACE;
    else
      return TokenData::STRING_LITERAL;
  }

  switch (type) {
    case SINGLE_LINE_COMMENT:
      return TokenData::LINE_COMMENT;

    case MULTI_LINE_COMMENT_START:
      return TokenData::BLOCK_COMMENT;

    case MULTI_LINE_COMMENT_END:
      error_.Log("block comment end without start", SOURCE_ERROR);
      return TokenData::UNKNOWN;

    case WHITESPACE:
      return TokenData::WHITESPACE;

    case LINE_BREAK:
    case NEWLINE:
      return TokenData::LINE_END;

    case OPERATOR:
      return TokenData::OPERATOR;

    case LETTER:
    case DIGIT:
      if (previous_type == TokenData::IDENTIFIER ||
          previous_type == TokenData::LITERAL)
        return previous_type;
      else if (type == DIGIT)
        return TokenData::LITERAL;
      else
        return TokenData::IDENTIFIER;

    case STRING_QUOTE:
      return TokenData::STRING_LITERAL;

    default:
      return TokenData::UNKNOWN;
  }
}

void LexString(std::shared_ptr<SourceFile> file, std::vector<Token>& tokens) {
  std::string token_txt;
  int line = 1;
  int char_pos = 1;

  TokenData::Type type = TokenData::WHITESPACE;

  for (int i = 0; i < file->Size(); i++) {
    CharacterClassifier::Type char_type = character_classifier.Get(file, i);
    TokenData::Type new_type =
        CharacterClassifier::GetTokenType(char_type, type);

    if (new_type != type) {
      if (!token_txt.empty()) {
        if (type == TokenData::OPERATOR) {
          std::vector<Operator> op_matches;
          ops_->Get(token_txt, op_matches);

          for (auto op : op_matches) {
            tokens.push_back(MakeToken(op->GetText(), file, line,
                                       char_pos - token_txt.size(), type, op));
          }
        } else if (type == TokenData::LINE_COMMENT ||
                   type == TokenData::BLOCK_COMMENT) {
          // NOOP
        } else {
          Token token = MakeToken(token_txt, file, line,
                                  char_pos - token_txt.size(), type);

          if (type == TokenData::UNKNOWN) {
            TerebinthError("invalid token '" + token_txt + "'", SOURCE_ERROR,
                           token)
                .Log();
          } else {
            tokens.push_back(token);
          }
        }
      }
      token_txt = "";
    }

    if (new_type != TokenData::WHITESPACE && new_type != TokenData::LINE_END) {
      if (new_type == TokenData::STRING_LITERAL && (*file)[i] == '\\') {
        i++;
        if ((*file)[i] == 'n')
          token_txt += '\n';
        else if ((*file)[i] == '"')
          token_txt += '"';
        else if ((*file)[i] == 't')
          token_txt += '\t';
        else if ((*file)[i] == '\\')
          token_txt += '\\';
        else
          throw TerebinthError(
              std::string() + "invalid escape character '\\" + (*file)[i] + "'",
              SOURCE_ERROR,
              MakeToken(token_txt + (*file)[i], file, line,
                        char_pos - token_txt.size(), type));
      } else {
        token_txt += (*file)[i];
      }
    }

    type = new_type;

    if ((*file)[i] == '\n') {
      line++;
      char_pos = 1;
    } else {
      char_pos++;
    }
  }
}
