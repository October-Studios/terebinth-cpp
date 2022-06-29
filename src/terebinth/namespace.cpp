#include "namespace.h"
#include <tuple>
#include "action.h"
#include "ast_node.h"
#include "stack_frame.h"
#include "string_utils.h"
#include "error_handler.h"
#include "string_num_conversion.h"
#include "type.h"

void NamespaceData::IdMap::Add(std::string key, AstNode node) {
  auto i = nodes_.find(key);

  if (i == nodes_.end()) {
    nodes_[key] = std::vector<AstNode>();
  }

  nodes_[key].push_back(std::move(node));
}

void NamespaceData::IdMap::Get(std::string key, std::vector<AstNodeBase*>& out) {
  auto matches = nodes_.find(key);

  if (matches != nodes_.end()) {
    for (auto i = 0; i < matches->second.size(); ++i) {
      out.push_back(&*matches->second[i]);
    }
  }
}

Namespace NamespaceData::MakeRootNamespace() {
  return Namespace(new NamespaceData(Namespace(nullptr), std::shared_ptr<StackFrame>(new StackFrame()), "root"));
}

Namespace NamespaceData::MakeChild() {
  return Namespace(new NamespaceData(shared_from_this(), stack_frame_));
}

Namespace NamespaceData::MakeChildAndFrame(std::string name_in) {
  return Namespace(new NamespaceData(shared_from_this(), std::shared_ptr<StackFrame>(new StackFrame()), name_in));
}

NamespaceData::NamespaceData(Namespace parent_in, std::shared_ptr<StackFrame> stack_frame_in, std::string name_in) {
  parent_ = parent_in;
  stack_frame_ = stack_frame_in;
  my_name_ = name_in;
}

std::string NamespaceData::GetString() { return "NamespaceData::GetString not yet implemented"; }

std::string NamespaceData::GetStringWithParents() {
  auto ptr = shared_from_this();

  std::string out;

  while (ptr) {
    out = str::PutStringInBox(ptr->GetString() + "\n" + out, ptr->my_name_);
    ptr = ptr->parent_;
  }

  return out;
}

void NamespaceData::SetInput(Type left, Type right) {
  if (parent_ && parent_->GetStackFrame() == stack_frame_) {
    error_.Log("called " + FUNC + " on namespace that is not the root of a stack frame, thus it cannot get input", INTERNAL_ERROR);
    return;
  }

  stack_frame_->SetInput(left, right);

  if (left->IsCreatable()) {
    std::string left_name = "me";
    size_t left_offset = stack_frame_->GetLeftOffset();
    Action left_get_action = VarGetAction(left_offset, left, left_name);
    Action left_set_action = VarSetAction(left_offset, left, left_name);
    AddNode(AstActionWrapper::Make(left_get_action), left_name);
    AddNode(AstActionWrapper::Make(left_set_action), left_name);
  }

  if (right->IsCreatable()) {
    std::string right_name = "in";
    size_t right_offset = stack_frame_->GetRightOffset();
    Action right_get_action = VarGetAction(right_offset, right, right_name);
    Action right_set_action = VarSetAction(right_offset, right, right_name);
    AddNode(AstActionWrapper::Make(right_get_action), right_name);
    AddNode(AstActionWrapper::Make(right_set_action), right_name);
  }
}

Action NamespaceData::AddVar(Type type, std::string name) {
  size_t offset = stack_frame_->GetSize();
  stack_frame_->AddMember(type);

  Action get_action;
  Action set_action;
  Action copy_action;

  Namespace top = shared_from_this();

  while(top->parent_) {
    top = top->parent_;
  }

  if (stack_frame_ != top->stack_frame_) {
    get_action = VarGetAction(offset, type, name);
    set_action = VarSetAction(offset, type, name);
  } else {
    get_action = GlobalGetAction(offset, type, name);
    set_action = GlobalSetAction(offset, type, name);
  }

  copy_action = GetCopier(type);

  if (copy_action) {
    dynamic_actions_.Add(name, AstActionWrapper::Make(BranchAction(void_action_, copy_action, get_action)));
  } else {
    dynamic_actions_.Add(name, AstActionWrapper::Make(get_action));
  }

  dynamic_actions_.Add(name, AstActionWrapper::Make(set_action));

  Action destructor = GetDestroyer(type);

  if (destructor) {
    destructor_actions_.push_back(BranchAction(void_action_, destructor, get_action));
  }

  return set_action;
}

void NamespaceData::AddNode(AstNode node, std::string id) {
  if (node->name_hint_.empty()) {
    node->name_hint_ = id;
    node->NameHintSet();
  }

  if (node->IsType()) {
    types_.Add(id, std::move(node));
  } else {
    if (node->CanBeWhatev()) {
      whatev_actions_.Add(id, std::move(node));
    } else {
      actions_.Add(id, std::move(node));
    }
  }
}

Type NamespaceData::GetType(std::string name, bool throw_source_error, Token token_for_error) {
  std::vector<AstNodeBase*> results;

  types_.Get(name, results);

  if (results.empty()) {
    if (parent_) {
      return parent_->GetType(name, throw_source_error, token_for_error);
    } else if (throw_source_error) {
      throw TerebinthError("'" + name + "' type not found", SOURCE_ERROR);
    } else {
      return nullptr;
    }
  } else if (results.size() != 1) {
    throw TerebinthError("namespace has multiple definitions of the same type '" + name + "'", INTERNAL_ERROR);
  } else if (results[0]->GetReturnType()->GetType() != TypeBase::METATYPE) {
    throw TerebinthError("node returning non meta type stored in namespace type map for type '" + name + "'", INTERNAL_ERROR);
  } else {
    return results[0]->GetReturnType()->GetSubType();
  }
}

Action NamespaceData::GetDestroyer(Type type) {
  return GetActionForTokenWithInput(MakeToken("__destroy__"), Void, type, false, false, nullptr);
}

Action NamespaceData::WrapInDestroyer(Action in) {
  Action destroyer = GetDestroyer(in->GetReturnType());

  return destroyer ? BranchAction(void_action_, destroyer, in) : in;
}

Action NamespaceData::GetCopier(Type type) {
  return GetActionForTokenWithInput(MakeToken("__copy__"), Void, type, false, false, nullptr);
}

Action NamespaceData::GetActionForTokenWithInput(Token token, Type left, Type right, bool dynamic, bool throw_source_error, Token token_for_error) {
  std::vector<Action> matches;
  std::vector<AstNodeBase*> nodes;

  bool found_nodes = false;
  AstNode tuple_node;

  std::string search_text = (token->GetOp() ? token->GetOp()->GetText() : token->GetText());

  GetNodes(nodes, search_text, true, dynamic, false);

  if (left->GetType() == TypeBase::TUPLE && token->GetType() == TokenData::IDENTIFIER) {
    auto match = left->GetSubType(search_text);

    if (match.type) {
      if (right->IsVoid()) {
        tuple_node = AstActionWrapper::Make(GetElemFromTupleAction(left, search_text));
        nodes.push_back(&*tuple_node);
      }
    }
  }

  if (!nodes.empty()) {
    found_nodes = true;
  }

  NodesToMatchingActions(matches, nodes, left, right);

  if (!matches.empty()) {
    if (matches.size() == 1) {
      return matches[0];
    } else if (throw_source_error) {
      throw TerebinthError("multiple matching instances of '" + token->GetText() + "' found", SOURCE_ERROR, token_for_error);
    } else {
      return nullptr;
    }
  }

  nodes.clear();
  GetNodes(nodes, search_text, false, false, true);

  if (!nodes.empty()) {
    found_nodes = true;
  }

  for (auto i: nodes) {
    AstNode instance = i->MakeCopyWithSpecificTypes(left, right);

    if (instance) {
      matches.push_back(instance->GetAction());
      actions_.Add(token->GetText(), std::move(instance));
    }
  }

  if (!matches.empty()) {
    if (matches.size() == 1) {
      return matches[0];
    } else if (throw_source_error) {
      throw TerebinthError("multiple whatev instances of '" + token->GetText() + "' found", SOURCE_ERROR, token_for_error);
    } else {
      return nullptr;
    }
  }

  if (!found_nodes && dynamic && token->GetType() == TokenData::IDENTIFIER && left->IsVoid() && right->IsCreatable()) {
    return AddVar(right, token->GetText());
  }

  if (throw_source_error) {
    if (found_nodes) {
      throw TerebinthError("correct overload of '" + token->GetText() + "' not found for types " + left->GetString() + " and " +right->GetString(), SOURCE_ERROR, token_for_error);
    } else {
      throw TerebinthError("'" + token->GetText() + "' not found", SOURCE_ERROR, token_for_error);
    }
  } else {
    return nullptr;
  }
}

void NamespaceData::GetNodes(std::vector<AstNodeBase*>& out, std::string text, bool check_actions, bool check_dynamic, bool check_whatev) {
  if (check_actions) {
    actions_.Get(text, out);
  }

  if (check_dynamic) {
    dynamic_actions_.Get(text, out);
  }

  if (check_whatev) {
    whatev_actions_.Get(text, out);
  }

  if (parent_) {
    parent_->GetNodes(out, text, check_actions, check_dynamic, check_whatev);
  }
}

void NamespaceData::NodesToMatchingActions(std::vector<Action>& out, std::vector<AstNodeBase*>& nodes, Type left_in_type, Type right_in_type) {
  for (auto i : nodes) {
    Action action = i->GetAction();

    if (action->GetInLeftType()->Matches(left_in_type) && action->GetInRightType()->Matches(right_in_type)) {
      out.push_back(action);
    }
  }
}
