#include "action.h"
#include "error_handler.h"
#include "namespace.h"
#include "token.h"

extern Namespace global_namespace_;

Action ResolveIntLiteral(Token token, Type type) {
  std::string in = token->GetText();
  int val = 0;

  for (auto i = in.begin(); i != in.end(); ++i) {
    if (*i < '0' || *i > '9') {
      error_.Log(std::string() + "bad character '" + *i +
                     "' found in number '" + in + "'",
                 SOURCE_ERROR, token);
      return nullptr;
    }

    val = val * 10 + (*i - '0');
  }

  if (type == Bool) {
    bool out = (val != 0);
    return ConstGetAction(&out, type, token->GetText(), global_namespace_);
  } else {
    int out = val;
    return ConstGetAction(&out, type, token->GetText(), global_namespace_);
  }
}

Action ResolveDoubleLiteral(Token token) {
  std::string in = token->GetText();

  double val = 0;
  int point_pos = 0;

  for (auto i = in.begin(); i != in.end(); ++i) {
    if (*i == '.' || *i == '_') {
      if (point_pos == 0) {
        point_pos = 10;
      } else {
        error_.Log(std::string() + "multiple decimal points found in number '" +
                       in + "'",
                   SOURCE_ERROR, token);
        return void_action_;
      }
    } else if (*i >= '0' && *i <= '9') {
      if (point_pos) {
        val += (double)(*i - '0') / point_pos;
        point_pos *= 10;
      } else {
        val = val * 10 + (*i - '0');
      }
    } else {
      error_.Log(std::string() + "bad character '" + *i +
                     "' found in number '" + in + "'",
                 SOURCE_ERROR, token);
      return void_action_;
    }
  }

  double out = val;
  return ConstGetAction(&out, Double, token->GetText(), global_namespace_);
}

std::string TbthStr2CppStr(void *obj);
void *CppStr2TbthStr(std::string cpp);

Action ResolveStringLiteral(Token token) {
  std::string text = token->GetText();

  while (text.size() > 0 && text[0] == '"') {
    text = text.substr(1, std::string::npos);
  }

  while (text.size() > 0 && text[text.size() - 1] == '"') {
    text = text.substr(0, text.size() - 1);
  }

  void *obj = CppStr2TbthStr(text);

  return ConstGetAction(obj, String, "\"" + text + "\"", global_namespace_);
}

Action ResolveLiteral(Token token) {
  if (token->GetType() == TokenData::STRING_LITERAL) {
    return ResolveStringLiteral(token);
  }

  if (token->GetType() != TokenData::LITERAL) {
    throw TerebinthError(FUNC + " called on token that is not a literal",
                         INTERNAL_ERROR, token);
  }

  std::string in = token->GetText();

  if (in.empty()) {
    return nullptr;
  }

  Type type = Unknown;

  if (in.empty()) {
    error_.Log("tried to make literal with empty string", INTERNAL_ERROR,
               token);
  }

  if ((in[0] >= '0' && in[0] <= '9') || in[0] == '.') {
    type = Int;

    for (auto i = in.begin(); i != in.end(); ++i) {
      if (*i == '.' || *i == '_') {
        type = Double;
        break;
      }
    }

    if (in.back() == 'd' || in.back() == 'f') {
      type = Double;
      in.pop_back();
    } else if (in.back() == 'i') {
      type = Int;
      in.pop_back();
    } else if (in.back() == 'b') {
      type = Bool;
      in.pop_back();
    }
  }

  if (type == Int) {
    return ResolveIntLiteral(token, type);
  } else if (type == Double) {
    return ResolveDoubleLiteral(token);
  } else {
    throw TerebinthError("tried to make literal with invalid type of " +
                             type->GetString(),
                         INTERNAL_ERROR, token);
  }
}
