module;
module Token;

import Util.StringUtils;

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

std::string TokenData::TypeToString(TokenData::Type in) {
  switch (in) {
    case WHITESPACE:
      return "whitespace";
    case LINE_END:
      return "newline";
    case IDENTIFIER:
      return "identifier";
    case LITERAL:
      return "literal";
    case STRING_LITERAL:
      return "string literal";
    case OPERATOR:
      return "operator";
    case LINE_COMMENT:
      return "single line comment";
    case BLOCK_COMMENT:
      return "block comment";
    case SCOPE:
      return "scope";
    case UNKNOWN:
      return "UNKNOWN";
    default:
      return "ERROR GETTING TOKEN TYPE";
  }
}

std::string TokenData::GetDescription() {
  return std::to_string(GetLine()) + ":" + std::to_string(GetCharPos()) + " (" +
         TokenData::TypeToString(token_type_) + " '" + GetText() + "')";
}

std::string TokenData::GetTypeDescription() {
  std::string out;

  if (token_type_ == TokenData::OPERATOR) {
    if (op_) {
      out += op_->GetText() + " ";
    } else {
      out += "unknown ";
    }
  }

  out += TokenData::TypeToString(token_type_);

  return out;
}

std::string TableStringFromTokens(const std::vector<Token>& tokens,
                                  std::string table_name) {
  std::vector<std::string> lines;
  std::string abv = "", blw = "";
  std::string str = "";
  const int max_width = 240;
  const std::string seperator = "    ";

  for (unsigned i = 0; i < tokens.size(); ++i) {
    if (i > 0 && str.size() + seperator.size() + tokens[i]->GetText().size() <
                     max_width) {
      abv += seperator;
      str += seperator;
      blw += seperator;

      for (unsigned j = 0; j < tokens[i]->GetText().size(); j++) {
        abv += " ";
        blw += " ";
      }

      str += tokens[i]->GetText();
    } else {
      if (i > 0) {
        lines.push_back(abv);
        lines.push_back(str);
        lines.push_back(blw);
        lines.push_back("");
        abv = "";
        blw = "";
      }

      for (unsigned j = 0; j < tokens[i]->GetText().size(); j++) {
        abv += " ";
        blw += " ";
      }

      str = tokens[i]->GetText();
    }
  }

  lines.push_back(abv);
  lines.push_back(str);
  lines.push_back(blw);

  return str::LineListToBoxedString(lines, table_name, -1, false,
                                    max_width + 4);
}

std::string StringFromTokens(const std::vector<Token>& tokens, int left,
                             int right) {
  std::string out;

  for (int i = left; i <= right; ++i) {
    out += tokens[i]->GetText();
    if (i < right) out += " ";
  }

  return out;
}
