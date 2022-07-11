#include "all_operators.h"
#include "ast_node.h"
#include "error_handler.h"
#include "namespace.h"
#include "stack_frame.h"
#include "string_utils.h"
#include "token.h"

#include <iostream>
#include <vector>

void LexString(std::shared_ptr<SourceFile> file, std::vector<Token> &tokens);

void ParseTokenList(const std::vector<Token> &tokens, int left, int right,
                    std::vector<AstNode> &nodes);

int FindExpressionSplit(const std::vector<Token> &tokens, int left, int right);

AstNode ParseExpression(const std::vector<Token> &tokens, int left, int right);

int SkipBrace(const std::vector<Token> &tokens, int start);

void ParseSequence(const std::vector<Token> &tokens, int left, int right,
                   Operator splitter, std::vector<AstNode> &out);

AstNode ParseOperator(const std::vector<Token> &tokens, int left, int right,
                      int index);

std::unique_ptr<AstType> ParseType(const std::vector<Token> &tokens, int left,
                                   int right);
void ImportFile(std::vector<AstNode> &nodes, std::string path);

AstNode AstNodeFromTokens(const std::vector<Token> &tokens, int left,
                          int right) {
  std::vector<AstNode> nodes;

  ParseTokenList(tokens, left, right, nodes);

  if (nodes.size() == 0) {
    return AstVoid::Make();
  } else if (nodes.size() == 1) {
    return std::move(nodes[0]);
  } else {
    return AstList::Make(nodes);
  }
}

int SkipBrace(const std::vector<Token> &tokens, int start) {
  Operator open, close;
  int step;

  Operator op = tokens[start]->GetOp();

  if (tokens[start]->GetOp() == ops_->open_paren_) {
    open = ops_->open_paren_;
    close = ops_->close_paren_;
    step = 1;
  } else if (tokens[start]->GetOp() == ops_->close_paren_) {
    open = ops_->close_paren_;
    close = ops_->open_paren_;
    step = -1;
  } else if (tokens[start]->GetOp() == ops_->open_sq_brac_) {
    open = ops_->open_sq_brac_;
    close = ops_->close_sq_brac_;
    step = 1;
  } else if (tokens[start]->GetOp() == ops_->close_sq_brac_) {
    open = ops_->close_sq_brac_;
    close = ops_->open_sq_brac_;
    step = -1;
  } else if (tokens[start]->GetOp() == ops_->open_cr_brac_) {
    open = ops_->open_cr_brac_;
    close = ops_->close_cr_brac_;
    step = 1;
  } else if (tokens[start]->GetOp() == ops_->close_cr_brac_) {
    open = ops_->close_cr_brac_;
    close = ops_->open_cr_brac_;
    step = -1;
  } else {
    throw TerebinthError(FUNC + " called with index that is not a valid brace",
                         INTERNAL_ERROR, tokens[start]);
  }

  int c = 1;
  int i = start;

  while (true) {
    i += step;

    if (i >= int(tokens.size())) {
      throw TerebinthError("no matching brace", SOURCE_ERROR, tokens[start]);
    }

    if (tokens[i]->GetOp() == open) {
      c++;
    } else if (tokens[i]->GetOp() == close) {
      c--;

      if (c <= 0) {
        return i;
      }
    }
  }
}

int FindExpressionSplit(const std::vector<Token> &tokens, int left, int right) {
  int min_prece = -1;
  int index_of_min = -1;

  for (int i = left; i <= right; ++i) {
    if (ops_->IsOpenBrac(tokens[i]->GetOp())) {
      int j = SkipBrace(tokens, i);

      i = j;
    } else if (tokens[i]->GetOp() &&
               (min_prece < 0 ||
                tokens[i]->GetOp()->GetPrecedence() < min_prece)) {
      min_prece = tokens[i]->GetOp()->GetPrecedence();
      index_of_min = i;

      if (min_prece % 2) {
        min_prece++;
      }
    }
  }

  if (index_of_min < 0) {
    throw TerebinthError(FUNC + " could not find operator to split expression",
                         INTERNAL_ERROR, tokens[left]);
  }

  return index_of_min;
}

AstNode ParseExpression(const std::vector<Token> &tokens, int left, int right) {
  if (left > right) {
    throw TerebinthError(FUNC + " sent left higher then right", INTERNAL_ERROR,
                         tokens[left]);
  } else if (left == right) {
    return AstToken::Make(tokens[left]);
  }

  if (ops_->IsOpenBrac(tokens[left]->GetOp()) &&
      SkipBrace(tokens, left) == right) {
    if (tokens[left]->GetOp() == ops_->open_paren_) {
      return AstNodeFromTokens(tokens, left + 1, right - 1);
    } else if (tokens[left]->GetOp() == ops_->open_cr_brac_) {
      return ParseType(tokens, left + 1, right - 1);
    } else {
      throw TerebinthError("unhandled brace '" +
                               tokens[left]->GetOp()->GetText() + "'",
                           INTERNAL_ERROR, tokens[left]);
    }
  }

  int i = FindExpressionSplit(tokens, left, right);

  Operator op = tokens[i]->GetOp();

  if (op == ops_->minus_ && i == left && i != right) {
    // NOOP
  } else if ((i == left) == op->TakesLeftInput() ||
             (i == right) == op->TakesRightInput()) {
    throw TerebinthError("improper use of '" + op->GetText() + "' operator",
                         SOURCE_ERROR, tokens[i]);
  }

  if (op == ops_->pipe_) {
    throw TerebinthError("invalid use of '" + op->GetText() + "'", SOURCE_ERROR,
                         tokens[i]);
  } else if (op == ops_->if_op_ || op == ops_->loop_ ||
             op == ops_->right_arrow_ || op == ops_->and_op_ ||
             op == ops_->or_op_) {
    std::vector<AstNode> left_nodes;
    std::vector<AstNode> right_nodes;

    if (i > left)
      ParseSequence(tokens, left, i - 1, ops_->pipe_, left_nodes);

    if (i < right)
      ParseSequence(tokens, i + 1, right, ops_->pipe_, right_nodes);

    return AstOpWithInput::Make(left_nodes, tokens[i], right_nodes);
  } else if (op == ops_->comma_) {
    std::vector<AstNode> nodes;

    ParseSequence(tokens, left, right, ops_->comma_, nodes);

    return AstTuple::Make(nodes);
  } else if (op == ops_->double_colon_) {
    std::unique_ptr<AstToken> center_node = nullptr;
    AstNode right_node = nullptr;

    if (i == left + 1 && tokens[i - 1]->GetType() == TokenData::IDENTIFIER) {
      center_node = AstToken::Make(tokens[i - 1]);
    } else {
      throw TerebinthError(
          "you can only use constant assignment on a single identifier",
          SOURCE_ERROR, tokens[i]);
    }

    if (i < right) {
      right_node = ParseExpression(tokens, i + 1, right);
    } else {
      right_node = AstVoid::Make();
    }

    return AstConstExpression::Make(std::move(center_node),
                                    std::move(right_node));
  } else if (op == ops_->not_equal_) {
    AstNode right_node =
        i < right ? ParseExpression(tokens, i + 1, right) : AstVoid::Make();
    AstNode left_node =
        i > left ? ParseExpression(tokens, left, i - 1) : AstVoid::Make();
    AstNode center_node = AstToken::Make(MakeToken(
        ops_->equal_->GetText(), tokens[i]->GetFile(), tokens[i]->GetLine(),
        tokens[i]->GetCharPos() + 1, TokenData::OPERATOR, ops_->equal_));

    AstNode not_node = AstToken::Make(MakeToken(
        ops_->not_op_->GetText(), tokens[i]->GetFile(), tokens[i]->GetLine(),
        tokens[i]->GetCharPos(), TokenData::OPERATOR, ops_->not_op_));

    return AstExpression::Make(AstVoid::Make(), std::move(not_node),
                               AstExpression::Make(std::move(left_node),
                                                   std::move(center_node),
                                                   std::move(right_node)));
  } else if (op == ops_->plus_plus_ || op == ops_->minus_minus_) {
    throw TerebinthError("++ and -- are not yet implemented", SOURCE_ERROR,
                         tokens[i]);
  } else if (op == ops_->dot_) {
    return AstExpression::Make(
        i > left ? ParseExpression(tokens, left, i - 1) : AstVoid::Make(),
        i < right ? ParseExpression(tokens, i + 1, right) : AstVoid::Make(),
        AstVoid::Make());
  } else if (op == ops_->colon_) {
    AstNode left_node = AstVoid::Make();
    AstNode center_node = ParseExpression(tokens, left, i - 1);
    AstNode right_node =
        i < right ? ParseExpression(tokens, i + 1, right) : AstVoid::Make();

    if (typeid(*center_node) == typeid(AstExpression)) {
      AstExpression *expr_node = (AstExpression *)&*center_node;

      if (!expr_node->left_in->IsVoid() && !expr_node->center->IsVoid() &&
          expr_node->right_in->IsVoid()) {
        left_node = std::move(expr_node->left_in);
        center_node = std::move(expr_node->center);
      }
    }

    // make a function body if needed, else make a normal expression

    if ((center_node->IsType() || center_node->IsFunctionWithOutput()) &&
        (left_node->IsVoid() || left_node->IsType()) && !right_node->IsType()) {
      if (left_node->IsVoid())
        left_node = AstVoidType::Make();

      AstNode func_right_in;
      AstNode func_return;

      if (center_node->IsFunctionWithOutput()) {
        func_right_in =
            std::move(((AstOpWithInput *)&*(center_node))->left_in[0]);
        func_return =
            std::move(((AstOpWithInput *)&*(center_node))->right_in[0]);
      } else {
        func_right_in = std::move(center_node);
        func_return = AstVoidType::Make();
      }

      return AstFuncBody::Make(std::move(left_node), std::move(func_right_in),
                               std::move(func_return), std::move(right_node));
    } else {
      return AstExpression::Make(std::move(left_node), std::move(center_node),
                                 std::move(right_node));
    }
  } else {
    return AstExpression::Make(
        i > left ? ParseExpression(tokens, left, i - 1) : AstVoid::Make(),
        AstToken::Make(tokens[i]),
        i < right ? ParseExpression(tokens, i + 1, right) : AstVoid::Make());
  }
}

void ParseTokenList(const std::vector<Token> &tokens, int left, int right,
                    std::vector<AstNode> &nodes) {
  int start = left;

  for (int i = left; i <= right; i++) {
    if (tokens[i]->GetOp() == ops_->import_) {
      if (i == right || tokens[i + 1]->GetType() != TokenData::STRING_LITERAL) {
        throw TerebinthError("'" + ops_->import_->GetText() +
                                 "' must be followed by a string literal",
                             SOURCE_ERROR, tokens[i]);
      }

      std::string path = tokens[i + 1]->GetText();

      while (path.size() > 0 && path[0] == '"') {
        path = path.substr(1, std::string::npos);
      }

      while (path.size() > 0 && path[path.size() - 1] == '"') {
        path = path.substr(0, path.size() - 1);
      }

      path = tokens[i]->GetFile()->GetDirPath() + "/" + path;

      try {
        ImportFile(nodes, path);
      } catch (TerebinthError err) {
        err.Log();
      }

      i += 2;
      start = i;
    } else {
      if (ops_->IsOpenBrac(tokens[i]->GetOp())) {
        i = SkipBrace(tokens, i);
      }

      bool token_takes_right_input =
          (tokens[i]->GetOp() && tokens[i]->GetOp()->TakesRightInput());
      int next = i + 1;
      bool next_token_takes_left_input =
          (next <= right && tokens[next]->GetOp() &&
           tokens[next]->GetOp()->TakesLeftInput());

      if (i == right ||
          (!token_takes_right_input && !next_token_takes_left_input)) {
        try {
          AstNode node = ParseExpression(tokens, start, i);
          nodes.push_back(std::move(node));
        } catch (TerebinthError err) {
          err.Log();
        }

        start = next;
      }
    }
  }
}

void ParseSequence(const std::vector<Token> &tokens, int left, int right,
                   Operator splitter, std::vector<AstNode> &out) {
  int start = left;

  for (int i = left; i <= right; i++) {
    if (ops_->IsOpenBrac(tokens[i]->GetOp())) {
      i = SkipBrace(tokens, i);
    } else if (tokens[i]->GetOp() == splitter) {
      if (start <= i - 1)
        out.push_back(ParseExpression(tokens, start, i - 1));
      start = i + 1;
    } else if (tokens[i]->GetOp() && tokens[i]->GetOp()->GetPrecedence() ==
                                         splitter->GetPrecedence()) {
      break;
    } else if (tokens[i]->GetOp() && tokens[i]->GetOp()->GetPrecedence() <
                                         splitter->GetPrecedence()) {
      out.clear();
      out.push_back(ParseExpression(tokens, left, right));
      return;
    }
  }

  if (start <= right)
    out.push_back(ParseExpression(tokens, start, right));
}

std::unique_ptr<AstType> ParseType(const std::vector<Token> &tokens, int left,
                                   int right) {
  std::vector<AstTupleType::NamedType> types;

  while (left <= right) {
    while (left <= right && tokens[left]->GetOp() == ops_->comma_) {
      left++;
    }

    if (left + 2 <= right && tokens[left + 1]->GetOp() == ops_->colon_) {
      if (tokens[left]->GetType() != TokenData::IDENTIFIER) {
        throw TerebinthError("identifier must be to the left of ':' in type",
                             SOURCE_ERROR, tokens[left]);
      }

      Token name = tokens[left];
      std::unique_ptr<AstType> type;

      if (tokens[left + 2]->GetType() == TokenData::IDENTIFIER) {
        type = AstTokenType::Make(tokens[left + 2]);
        left += 3;
      } else if (tokens[left + 2]->GetOp() == ops_->open_cr_brac_) {
        int j = SkipBrace(tokens, left + 2);

        if (j > right) {
          throw TerebinthError(FUNC + " skipping brance went outside of range",
                               INTERNAL_ERROR, tokens[left + 1]);
        }

        type = ParseType(tokens, left + 2 + 1, j - 1);
        left = j + 1;
      } else {
        throw TerebinthError("invalid thingy '" + tokens[left + 2]->GetText() +
                                 "' in type",
                             SOURCE_ERROR, tokens[left + 2]);
      }

      types.push_back(AstTupleType::NamedType{name, std::move(type)});
    } else {
      types.push_back(
          AstTupleType::NamedType{nullptr, AstTokenType::Make(tokens[left])});
      left += 1;
    }
  }

  if (types.size() == 0)
    return AstVoidType::Make();
  else if (types.size() == 1 && !types[0].name)
    return std::move(types[0].type);
  else
    return AstTupleType::Make(types);
}

void ImportFile(std::vector<AstNode> &nodes, std::string path) {
  auto file = std::shared_ptr<SourceFile>(new SourceFile(path, false));

  if (file->GetContents().empty()) {
    throw TerebinthError("file '" + path + "' failed to open or was empty",
                         SOURCE_ERROR);
  }

  std::vector<Token> tokens;

  LexString(file, tokens);

  ParseTokenList(tokens, 0, tokens.size() - 1, nodes);
}
