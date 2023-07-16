#include "ast_node.h"

#include "all_operators.h"
#include "error_handler.h"
#include "namespace.h"
#include "util/string_drawing.h"
#include "util/string_utils.h"

extern StackFrame std_lib_stack_frame_;
extern Namespace global_namespace_;

auto ResolveLiteral(Token token) -> Action;

void AstNodeBase::CopyToNode(AstNodeBase* other, bool copy_cache) {
  other->in_left_type = in_left_type;
  other->in_right_type = in_right_type;
  other->name_hint_ = name_hint_;

  if (copy_cache) {
    other->action_ = action_;
    other->return_type_ = return_type_;
    other->dynamic_ = dynamic_;
    other->ns_ = ns_;
    other->input_has_been_set_ = input_has_been_set_;
  }
}

std::string AstList::GetString() {
  std::vector<std::string> data;

  for (auto i = 0; i < int(nodes.size()); ++i) {
    data.push_back(nodes[i]->GetString());
  }

  return str::MakeList(data);
}

void AstList::ResolveAction() {
  if (!in_left_type->IsVoid() || !in_right_type->IsVoid()) {
    throw TerebinthError("AstList given non void input", INTERNAL_ERROR,
                         GetToken());
  }

  ns_ = ns_->MakeChild();

  for (auto i = 0; i < int(nodes.size()); ++i) {
    nodes[i]->SetInput(ns_, dynamic_, Void, Void);
    nodes[i]->DealWithConstants();
  }

  std::vector<Action> actions;

  for (auto i = 0; i < int(nodes.size()); ++i) {
    try {
      Action action = nodes[i]->GetAction();

      if (i != (int)nodes.size() - 1) action = ns_->WrapInDestroyer(action);

      actions.push_back(action);
    } catch (TerebinthError err) {
      err.Log();
    }
  }

  action_ = ListActionT(actions, *ns_->GetDestroyerActions());
}

std::string AstFuncBody::GetString() {
  std::vector<std::string> data;
  data.push_back("function");

  std::vector<std::string> types = {left_type_node->GetString(),
                                    right_type_node->GetString(),
                                    return_type_node->GetString()};

  data.push_back(str::MakeList(types));
  data.push_back(body_node_->GetString());

  return str::MakeList(data);
}

AstNode AstFuncBody::MakeCopyWithSpecificTypes(Type left_in_type,
                                               Type right_in_type) {
  Type left_whatev_type = left_type_node->GetReturnType()->GetSubType();
  Type right_whatev_type = right_type_node->GetReturnType()->GetSubType();

  if (!left_in_type->Matches(left_whatev_type) ||
      !right_in_type->Matches(right_whatev_type)) {
    return nullptr;
  }

  AstNode actual_left_type_node;
  AstNode actual_right_type_node;

  if (left_whatev_type->IsWhatev()) {
    actual_left_type_node =
        AstTypeType::Make(left_whatev_type->ActuallyIs(left_in_type));
  } else {
    actual_left_type_node = left_type_node->MakeCopy(false);
  }

  if (right_whatev_type->IsWhatev()) {
    actual_right_type_node =
        AstTypeType::Make(right_whatev_type->ActuallyIs(right_in_type));
  } else {
    actual_right_type_node = right_type_node->MakeCopy(false);
  }

  AstNode out =
      Make(std::move(actual_left_type_node), std::move(actual_right_type_node),
           return_type_node->MakeCopy(false), body_node_->MakeCopy(false));
  out->name_hint_ = name_hint_;
  out->SetInput(ns_, dynamic_, Void, Void);
  return out;
}

void AstFuncBody::ResolveAction() {
  SetTypesInput();
  Namespace sub_ns =
      ns_->MakeChildAndFrame(name_hint_.empty() ? "unnamed function's namespace"
                                                : name_hint_ + "'s namespace");
  sub_ns->SetInput(left_type_node->GetReturnType()->GetSubType(),
                   right_type_node->GetReturnType()->GetSubType());
  body_node_->SetInput(sub_ns, true, Void, Void);
  Type func_return_type = return_type_node->GetReturnType()->GetSubType();
  if (func_return_type->IsWhatev()) {
    func_return_type =
        func_return_type->ActuallyIs(body_node_->GetReturnType());
  }
  action_ = FunctionActionT(body_node_->MakeCopy(true), func_return_type,
                            sub_ns->GetStackFrame());
}

std::string AstExpression::GetString() {
  std::string left_str;
  if (!left_in->IsVoid()) {
    left_str = left_in->GetString();
  }

  std::string center_str = center->GetString();

  std::string right_str;
  if (!right_in->IsVoid()) {
    right_str = right_in->GetString();
  }

  return str::MakeRootUpBinaryTree(center_str, "", "", left_str, right_str);
}

void AstExpression::ResolveAction() {
  if (!in_left_type->IsVoid() || !in_right_type->IsVoid()) {
    throw TerebinthError("AstExpression given non void input", INTERNAL_ERROR,
                         GetToken());
  }

  if (right_in->IsType()) {
    throw TerebinthError("types must be declared as constants", SOURCE_ERROR,
                         right_in->GetToken());
  } else if (center->IsType() || center->IsFunctionWithOutput() ||
             left_in->IsType()) {
    throw TerebinthError(
        "a function implementation got into an expression node somehow",
        INTERNAL_ERROR, center->GetToken());
  } else {
    left_in->SetInput(ns_, dynamic_, Void, Void);
    right_in->SetInput(ns_, dynamic_, Void, Void);
    center->SetInput(ns_, dynamic_, left_in->GetReturnType(),
                     right_in->GetReturnType());
    action_ = BranchActionT(left_in->GetAction(), center->GetAction(),
                            right_in->GetAction());
  }

  if (action_->name_hint_.empty()) action_->name_hint_ = name_hint_;
}

std::string AstConstExpression::GetString() {
  std::string center_str = center->GetString();

  std::string right_str;
  if (!right_in->IsVoid()) {
    right_str = right_in->GetString();
  }

  return str::MakeRootUpBinaryTree(center_str, "", "const", "", right_str);
}

void AstConstExpression::ResolveConstant() {
  if (!in_left_type->IsVoid() || !in_right_type->IsVoid()) {
    throw TerebinthError("AstConstExpression given non void input",
                         INTERNAL_ERROR, GetToken());
  }

  right_in->SetInput(ns_, false, Void, Void);
  ns_->AddNode(std::move(right_in->MakeCopy(true)), center->token->GetText());
}

std::string AstOpWithInput::GetString() {
  std::string left;
  std::vector<std::string> data;
  for (auto i = 0; i < int(left_in.size()); ++i) {
    data.push_back(left_in[i]->GetString());
  }

  if (data.size() == 1) {
    left = data[0];
  } else if (data.size() > 1) {
    left = str::MakeList(data);
  }

  data.clear();

  std::string right;
  for (auto i = 0; i < int(right_in.size()); ++i) {
    data.push_back(right_in[i]->GetString());
  }

  if (data.size() == 1) {
    right = data[0];
  } else if (data.size() > 1) {
    right = str::MakeList(data);
  }

  return str::MakeRootUpBinaryTree(
      str::PutStringInTreeNodeBox(token->GetText()), "", "", left, right);
}

void AstOpWithInput::ResolveAction() {
  if (token->GetOp() == ops_->if_op_) {
    for (auto i = 0; i < int(left_in.size()); ++i)
      left_in[i]->SetInput(ns_, dynamic_, Void, Void);

    for (auto i = 0; i < int(right_in.size()); ++i)
      right_in[i]->SetInput(ns_, dynamic_, Void, Void);

    if (left_in.empty()) {
      throw TerebinthError("'?' must have a conditional to its left",
                           SOURCE_ERROR, token);
    } else if (left_in.size() != 1) {
      throw TerebinthError("'?' can only have one conditional to its left",
                           SOURCE_ERROR, token);
    }

    Action condition = left_in[0]->GetAction();

    if (right_in.empty()) {
      throw TerebinthError("'?' must have a statement to its right",
                           SOURCE_ERROR, token);
    } else if (right_in.size() <= 2) {
      Action a;

      try {
        a = right_in[0]->GetAction();
      } catch (TerebinthError err) {
        err.Log();
        a = void_action_;
      }

      if (right_in.size() == 1) {
        action_ = IfActionT(condition, a);
      } else {
        Action e;

        try {
          e = right_in[1]->GetAction();
        } catch (TerebinthError err) {
          err.Log();
          e = void_action_;
        }

        action_ = IfElseActionT(condition, a, e);
      }
    } else {
      throw TerebinthError(
          "'?' can only have 1 or 2 '|' seporated expressions to its right",
          SOURCE_ERROR, token);
    }
  } else if (token->GetOp() == ops_->loop_) {
    bool uses_sub_ns = false;

    if (left_in.size() == 3) {
      ns_ = ns_->MakeChild();
      uses_sub_ns = true;
    }

    for (auto i = 0; i < int(left_in.size()); ++i)
      left_in[i]->SetInput(ns_, dynamic_, Void, Void);

    for (auto i = 0; i < int(right_in.size()); ++i)
      right_in[i]->SetInput(ns_, dynamic_, Void, Void);

    Action init_action = nullptr, condition_action, end_action, body_action;

    if (right_in.size() > 1) {
      throw TerebinthError("'@' followed by multiple expressions", SOURCE_ERROR,
                           token);
    }

    if (left_in.size() == 0) {
      throw TerebinthError("condition needed before '@'", SOURCE_ERROR, token);
    } else if (left_in.size() == 1) {
      condition_action = left_in[0]->GetAction();
      end_action = void_action_;
    } else if (left_in.size() == 2) {
      condition_action = left_in[0]->GetAction();
      end_action = left_in[1]->GetAction();
    } else if (left_in.size() == 3) {
      init_action = left_in[0]->GetAction();
      condition_action = left_in[1]->GetAction();
      end_action = left_in[2]->GetAction();
    } else {
      throw TerebinthError("chain of length " + std::to_string(left_in.size()) +
                               "preceding '@', it should be length 1-3",
                           SOURCE_ERROR, token);
    }

    if (right_in.empty()) {
      body_action = void_action_;
    } else {
      try {
        body_action = right_in[0]->GetAction();
      } catch (TerebinthError err) {
        err.Log();
        body_action = void_action_;
      }
    }

    std::vector<Action> actions;

    if (init_action) actions.push_back(ns_->WrapInDestroyer(init_action));

    actions.push_back(ns_->WrapInDestroyer(
        LoopActionT(condition_action, end_action, body_action)));

    action_ = uses_sub_ns
                  ? action_ = ListActionT(actions, *ns_->GetDestroyerActions())
                  : action_ = ListActionT(actions, {});
  } else if (token->GetOp() == ops_->and_op_ ||
             token->GetOp() == ops_->or_op_) {
    if (left_in.size() > 1 || right_in.size() > 1) {
      throw TerebinthError("'" + token->GetOp()->GetText() +
                               "' can not be sent '|' separated sequence",
                           SOURCE_ERROR, GetToken());
    }

    if (left_in.size() != 1 || right_in.size() != 1) {
      throw TerebinthError("'" + token->GetOp()->GetText() +
                               "' must be given a left and right input",
                           SOURCE_ERROR, GetToken());
    }

    left_in[0]->SetInput(ns_, dynamic_, Void, Void);
    right_in[0]->SetInput(ns_, dynamic_, Void, Void);

    Action left_action = left_in[0]->GetAction();
    Action right_action = right_in[0]->GetAction();

    if (left_action->GetReturnType() != Bool) {
      throw TerebinthError(
          "'" + token->GetOp()->GetText() + "' can only be used with Bools",
          SOURCE_ERROR, left_in[0]->GetToken());
    }

    if (right_action->GetReturnType() != Bool) {
      throw TerebinthError(
          "'" + token->GetOp()->GetText() + "' can only be used with Bools",
          SOURCE_ERROR, right_in[0]->GetToken());
    }

    if (token->GetOp() == ops_->and_op_) {
      action_ = AndActionT(left_action, right_action);
    } else {
      action_ = OrActionT(left_action, right_action);
    }
  } else if (token->GetOp() == ops_->right_arrow_) {
    throw TerebinthError("AstOpWithInput::resolveAction called for token '" +
                             token->GetOp()->GetText() +
                             "', which it shouldn't have been",
                         INTERNAL_ERROR, token);
  } else {
    throw TerebinthError(
        "AstOpWithInput made with bad token '" + token->GetText() + "'",
        INTERNAL_ERROR, token);
  }
}

bool AstOpWithInput::IsFunctionWithOutput() {
  return token->GetOp() == ops_->right_arrow_ && left_in.size() == 1 &&
         right_in.size() == 1;
}

std::string AstToken::GetString() {
  return str::PutStringInTreeNodeBox(token->GetText());
}

void AstToken::ResolveAction() {
  if (token->GetType() == TokenData::IDENTIFIER ||
      token->GetType() == TokenData::OPERATOR) {
    if (token->GetOp() && !token->GetOp()->IsOverloadable()) {
      throw TerebinthError(
          "non overloadable operator in AstToken, it should "
          "have been removed and processed by the parser",
          INTERNAL_ERROR, token);
    }

    action_ = ns_->GetActionForTokenWithInput(
        token, in_left_type, in_right_type, dynamic_, true, token);
  } else if (token->GetType() == TokenData::LITERAL ||
             token->GetType() == TokenData::STRING_LITERAL) {
    if (!in_left_type->IsVoid() || !in_right_type->IsVoid()) {
      throw TerebinthError("a literal can not be given an input", SOURCE_ERROR,
                           token);
    }

    action_ = ResolveLiteral(token);
  } else {
    throw TerebinthError("AstToken givin invalid token '" + token->GetText() +
                             "' of type " +
                             TokenData::TypeToString(token->GetType()),
                         INTERNAL_ERROR, token);
  }
}

std::string AstTuple::GetString() {
  std::vector<std::string> data;

  for (auto i = 0; i < int(nodes.size()); ++i) {
    data.push_back(nodes[i]->GetString());
  }

  return str::MakeList(data);
}

void AstTuple::ResolveAction() {
  std::vector<Action> actions;

  for (auto i = 0; i < int(nodes.size()); ++i) {
    nodes[i]->SetInput(ns_, dynamic_, Void, Void);
    actions.push_back(nodes[i]->GetAction());
  }

  action_ = MakeTupleActionT(actions);
}

std::string AstTokenType::GetString() { return "{" + token->GetText() + "}"; }

void AstTokenType::ResolveReturnType() {
  return_type_ = ns_->GetType(token->GetText(), true, token)->GetMeta();
}

std::string AstTupleType::GetString() {
  std::string out;

  out += "AstTupleType{";

  for (auto i = 0; i < int(subTypes.size()); ++i) {
    auto type = &subTypes[i];

    if (type->name) {
      out += type->name->GetText() + ": ";
    }

    out += type->type->GetString();

    if (i < int(subTypes.size()) - 1) {
      out += ", ";
    }
  }

  out += "}";

  return out;
}

void AstTupleType::ResolveReturnType() {
  TupleTypeMaker maker;

  for (unsigned i = 0; i < subTypes.size(); ++i) {
    subTypes[i].type->SetInput(ns_, false, Void, Void);

    if (subTypes[i].name) {
      maker.Add(subTypes[i].name->GetText(),
                subTypes[i].type->GetReturnType()->GetSubType());
    } else {
      maker.Add(subTypes[i].type->GetReturnType()->GetSubType());
    }
  }

  return_type_ = maker.Get(true)->GetMeta();
}
