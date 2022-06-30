#ifndef TEREBINTH_AST_NODE_H_
#define TEREBINTH_AST_NODE_H_

#include "action.h"
#include "error_handler.h"
#include "token.h"
#include <memory>

class NamespaceData;
typedef std::shared_ptr<NamespaceData> Namespace;

#include <vector>

class AstNodeBase;
typedef std::unique_ptr<AstNodeBase> AstNode;

AstNode AstNodeFromTokens(const std::vector<Token> &tokens, int left,
                          int right);

class AstNodeBase {
public:
  virtual ~AstNodeBase() = default;

  void SetInput(Namespace ns_in, bool dynamic_in, Type left, Type right) {
    if (input_has_been_set_) {
      throw TerebinthError("tried to set input on an AST node '" + GetString() +
                               "' more than once",
                           INTERNAL_ERROR, GetToken());
    }

    input_has_been_set_ = true;

    ns_ = ns_in;
    dynamic_ = dynamic_in;
    in_left_type = left;
    in_right_type = right;
  }

  virtual bool IsVoid() { return false; }
  virtual bool IsType() { return false; }
  virtual bool IsFunctionWithOutput() { return false; }

  virtual std::string GetString() = 0;
  virtual AstNode MakeCopy(bool copy_cache) = 0;
  virtual bool CanBeWhatev() { return false; }

  virtual AstNode MakeCopyWithSpecificTypes(Type left_in_type,
                                            Type right_in_type) {
    return nullptr;
  }

  Type GetReturnType() {
    if (!return_type_) {
      if (!input_has_been_set_) {
        throw TerebinthError("tried to get return type from AST node when "
                             "input had not been set",
                             INTERNAL_ERROR, GetToken());
      }
      ResolveReturnType();
    }

    if (!return_type_) {
      throw TerebinthError("AST node " + GetString() +
                               "failed to supply a return type",
                           INTERNAL_ERROR);
    }

    return return_type_;
  }

  Action GetAction() {
    if (!action_) {
      if (!ns_) {
        throw TerebinthError(
            "tried to get action from AST node when input had not been set",
            INTERNAL_ERROR, GetToken());
      }

      ResolveAction();

      if (!name_hint_.empty()) {
        action_->name_hint_ = name_hint_;
      }
    }

    return action_;
  }

  void DealWithConstants() {
    if (!ns_) {
      throw TerebinthError("tried to deal with constants before input was set",
                           INTERNAL_ERROR, GetToken());
    }

    ResolveConstant();
  }

  virtual Token GetToken() = 0;
  std::string name_hint_ = "";
  virtual void NameHintSet() {}

protected:
  AstNodeBase() {}

  void CopyToNode(AstNodeBase *other, bool copy_cache);

  virtual void ResolveReturnType() {
    return_type_ = GetAction()->GetReturnType();
  }

  virtual void ResolveAction() = 0;
  virtual void ResolveConstant(){};

  Type in_left_type = nullptr, in_right_type = nullptr;
  Action action_ = nullptr;
  Type return_type_ = nullptr;
  bool dynamic_ = false;
  Namespace ns_ = nullptr;
  bool input_has_been_set_ = false;
};

class AstVoid : public AstNodeBase {
public:
  static std::unique_ptr<AstVoid> Make() {
    return std::unique_ptr<AstVoid>(new AstVoid);
  }

  bool IsVoid() { return true; }

  std::string GetString() { return "void node"; }

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstVoid;
    CopyToNode(out, copy_cache);
    return AstNode(out);
  }

  void ResolveReturnType() { return_type_ = Void; }

  void ResolveAction() {
    if (!in_left_type->IsVoid() || !in_right_type->IsVoid()) {
      throw TerebinthError("AstVoid given non void input", INTERNAL_ERROR,
                           GetToken());
    }

    action_ = void_action_;
  }

  Token GetToken() { return nullptr; }
};
// extern AstNode astVoid;

class AstList : public AstNodeBase {
public:
  //	Make a new instance of this type of node
  static std::unique_ptr<AstList> Make(std::vector<AstNode> &in) {
    std::unique_ptr<AstList> node(new AstList);
    node->nodes = std::move(in);
    return node;
  }

  std::string GetString();

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstList;
    CopyToNode(out, copy_cache);
    for (int i = 0; i < (int)nodes.size(); i++) {
      out->nodes.push_back(nodes[i]->MakeCopy(copy_cache));
    }
    return AstNode(out);
  }

  void ResolveAction();

  Token GetToken() { return nodes.empty() ? nullptr : nodes[0]->GetToken(); }

private:
  //	the list of sub nodes
  std::vector<AstNode> nodes;
};

class AstConstExpression;

class AstToken : public AstNodeBase {
public:
  static std::unique_ptr<AstToken> Make(Token token_in) {
    std::unique_ptr<AstToken> node(new AstToken);
    node->token = token_in;
    return node;
  }

  std::string GetString();

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstToken;
    CopyToNode(out, copy_cache);
    out->token = token;
    return AstNode(out);
  }

  void ResolveAction();

  Token GetToken() { return token; }

private:
  friend AstConstExpression;

  Token token = nullptr;
};

class AstFuncBody : public AstNodeBase {
public:
  static AstNode Make(AstNode left_type_in, AstNode right_type_in,
                      AstNode return_type_in, AstNode body_in) {
    auto node = new AstFuncBody();

    if (!left_type_in->IsType() || !right_type_in->IsType() ||
        !return_type_in->IsType()) {
      throw TerebinthError(
          "AstFuncBody made with function input nodes that are not types",
          INTERNAL_ERROR);
    }

    node->left_type_node = std::move(left_type_in);
    node->right_type_node = std::move(right_type_in);
    node->return_type_node = std::move(return_type_in);
    node->body_node_ = std::move(body_in);

    return AstNode(node);
  }

  std::string GetString();

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstFuncBody;
    CopyToNode(out, copy_cache);
    out->left_type_node = left_type_node->MakeCopy(copy_cache);
    out->right_type_node = right_type_node->MakeCopy(copy_cache);
    out->return_type_node = return_type_node->MakeCopy(copy_cache);
    out->body_node_ = body_node_->MakeCopy(copy_cache);
    out->types_input_set = types_input_set;
    return AstNode(out);
  }

  void ResolveAction();

  AstNode MakeCopyWithSpecificTypes(Type left_in_type, Type right_in_type);

  Token GetToken() { return body_node_->GetToken(); }

  void SetTypesInput() {
    if (!types_input_set) {
      left_type_node->SetInput(ns_, false, Void, Void);
      right_type_node->SetInput(ns_, false, Void, Void);
      return_type_node->SetInput(ns_, false, Void, Void);
      types_input_set = true;
    }
  }

  bool CanBeWhatev() {
    SetTypesInput();

    return left_type_node->GetReturnType()->IsWhatev() ||
           right_type_node->GetReturnType()->IsWhatev() ||
           return_type_node->GetReturnType()->IsWhatev();
  }

  AstNode left_type_node, right_type_node, return_type_node, body_node_;
  bool types_input_set = false;
};

class AstExpression : public AstNodeBase {
public:
  static std::unique_ptr<AstExpression>
  Make(AstNode left_in_in, AstNode center_in, AstNode right_in_in) {
    std::unique_ptr<AstExpression> node(new AstExpression);

    node->left_in = std::move(left_in_in);
    node->center = std::move(center_in);
    node->right_in = std::move(right_in_in);

    return node;
  }

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstExpression;
    CopyToNode(out, copy_cache);
    out->left_in = left_in->MakeCopy(copy_cache);
    out->center = center->MakeCopy(copy_cache);
    out->right_in = right_in->MakeCopy(copy_cache);
    return AstNode(out);
  }

  std::string GetString();

  void ResolveAction();

  Token GetToken() { return center->GetToken(); }

  AstNode left_in = nullptr, center = nullptr, right_in = nullptr;
};

class AstConstExpression : public AstNodeBase {
public:
  static std::unique_ptr<AstConstExpression>
  Make(std::unique_ptr<AstToken> center_in, AstNode right_in_in) {
    std::unique_ptr<AstConstExpression> node(new AstConstExpression);

    node->center = std::move(center_in);
    node->right_in = std::move(right_in_in);

    return node;
  }

  std::string GetString();

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstConstExpression;
    CopyToNode(out, copy_cache);
    out->center = std::unique_ptr<AstToken>(
        (AstToken *)center->MakeCopy(copy_cache).release());
    out->right_in = center->MakeCopy(copy_cache);
    return AstNode(out);
  }

  void ResolveConstant();
  void ResolveAction() { action_ = void_action_; };

  Token GetToken() { return center->GetToken(); }

private:
  std::unique_ptr<AstToken> center = nullptr;
  AstNode right_in = nullptr;
};

class AstOpWithInput : public AstNodeBase {
public:
  static std::unique_ptr<AstOpWithInput> Make(std::vector<AstNode> &left_in_in,
                                              Token token_in,
                                              std::vector<AstNode> &right_in_in) {
    std::unique_ptr<AstOpWithInput> node(new AstOpWithInput);

    node->left_in = std::move(left_in_in);
    node->token = token_in;
    node->right_in = std::move(right_in_in);

    return node;
  }

  bool IsFunctionWithOutput();

  std::string GetString();

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstOpWithInput;
    CopyToNode(out, copy_cache);
    out->token = token;
    for (int i = 0; i < (int)left_in.size(); i++) {
      out->left_in.push_back(left_in[i]->MakeCopy(copy_cache));
    }
    for (int i = 0; i < (int)right_in.size(); i++) {
      out->right_in.push_back(right_in[i]->MakeCopy(copy_cache));
    }
    return AstNode(out);
  }

  void ResolveAction();

  Token GetToken() { return token; }

  Token token = nullptr;
  std::vector<AstNode> left_in, right_in;
};

class AstTuple : public AstNodeBase {
public:
  //	Make a new instance of this type of node
  static std::unique_ptr<AstTuple> Make(std::vector<AstNode> &in) {
    std::unique_ptr<AstTuple> node(new AstTuple);
    node->nodes = std::move(in);
    return node;
  }

  std::string GetString();

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstTuple;
    CopyToNode(out, copy_cache);
    for (int i = 0; i < (int)nodes.size(); i++) {
      out->nodes.push_back(nodes[i]->MakeCopy(copy_cache));
    }
    return AstNode(out);
  }

  void ResolveAction();

  Token GetToken() { return nodes.empty() ? nullptr : nodes[0]->GetToken(); }

private:
  std::vector<AstNode> nodes;
};

class AstType : public AstNodeBase {
public:
  bool IsType() { return true; }

  void ResolveAction() {
    throw TerebinthError(
        "AstType::resolveAction called, which it shouldn't have been",
        INTERNAL_ERROR, GetToken());
  }
};

class AstTypeType : public AstType {
public:
  static std::unique_ptr<AstTypeType> Make(Type typeIn) {
    std::unique_ptr<AstTypeType> node(new AstTypeType);
    node->return_type_not_meta = typeIn;
    return node;
  }

  std::string GetString() { return return_type_->GetString(); }

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstTypeType;
    CopyToNode(out, copy_cache);
    out->return_type_ = return_type_;
    return AstNode(out);
  }

  void ResolveReturnType() { return_type_ = return_type_not_meta->GetMeta(); }

  void NameHintSet() {
    if (!name_hint_.empty() && return_type_not_meta->name_hint_.empty())
      return_type_not_meta->name_hint_ = name_hint_;
  }

  Token GetToken() { return nullptr; }

private:
  Type return_type_not_meta;
};

class AstVoidType : public AstType {
public:
  static std::unique_ptr<AstVoidType> Make() {
    std::unique_ptr<AstVoidType> node(new AstVoidType);
    return node;
  }

  std::string GetString() { return "{}"; }

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstVoidType;
    CopyToNode(out, copy_cache);
    return AstNode(out);
  }

  void ResolveReturnType() { return_type_ = Void->GetMeta(); }

  Token GetToken() { return nullptr; }

private:
};

class AstTokenType : public AstType {
public:
  static std::unique_ptr<AstTokenType> Make(Token tokenIn) {
    std::unique_ptr<AstTokenType> node(new AstTokenType);
    node->token = tokenIn;
    return node;
  }

  std::string GetString();

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstTokenType;
    CopyToNode(out, copy_cache);
    out->token = token;
    return AstNode(out);
  }

  void ResolveReturnType();

  Token GetToken() { return token; }

private:
  Token token = nullptr;
};

class AstTupleType : public AstType {
public:
  struct NamedType {
    Token name; // can be null
    std::unique_ptr<AstType> type;
  };

  static std::unique_ptr<AstTupleType> Make(std::vector<NamedType> &in) {
    std::unique_ptr<AstTupleType> node(new AstTupleType);
    node->subTypes = std::move(in);
    return node;
  }

  std::string GetString();

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstTupleType;
    CopyToNode(out, copy_cache);
    for (int i = 0; i < (int)subTypes.size(); i++) {
      out->subTypes.push_back(
          {subTypes[i].name,
           std::unique_ptr<AstType>(
               (AstType *)subTypes[i].type->MakeCopy(copy_cache).release())});
    }
    return AstNode(out);
  }

  void ResolveReturnType();

  Token GetToken() { return subTypes.empty() ? nullptr : subTypes[0].name; }

private:
  std::vector<NamedType> subTypes;
};

class AstActionWrapper : public AstNodeBase {
public:
  static std::unique_ptr<AstActionWrapper> Make(Action actionIn) {
    auto out = std::unique_ptr<AstActionWrapper>(new AstActionWrapper);
    out->in_left_type = actionIn->GetInLeftType();
    out->in_right_type = actionIn->GetInRightType();
    out->return_type_ = actionIn->GetReturnType();
    out->action_ = actionIn;
    out->dynamic_ = true;
    out->ns_ = nullptr;
    out->input_has_been_set_ = true;
    return out;
  }

  std::string GetString() { return "action wrapper node"; }

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstActionWrapper;
    CopyToNode(out, true);
    return AstNode(out);
  }

  void ResolveAction() {
    throw TerebinthError(
        "AstActionWrapper::resolveAction called, which it shouldn't have been",
        INTERNAL_ERROR);
  }

  Token GetToken() { return nullptr; }
};

class AstWhatevToActionFactory : public AstNodeBase {
public:
  static AstNode Make(std::function<Action(Type left, Type right)> lambda) {
    auto node = new AstWhatevToActionFactory();
    node->lambda = lambda;
    return AstNode(node);
  }

  std::string GetString() { return "AstWhatevToActionFactory"; }

  AstNode MakeCopy(bool copy_cache) {
    auto out = new AstWhatevToActionFactory;
    CopyToNode(out, copy_cache);
    out->lambda = lambda;
    return AstNode(out);
  }

  void ResolveAction() {
    throw TerebinthError("AstWhatevToActionFactory::resolveAction called, wich "
                         "should never happen",
                         INTERNAL_ERROR);
  }

  AstNode MakeCopyWithSpecificTypes(Type left_in_type, Type right_in_type) {
    auto action = lambda(left_in_type, right_in_type);
    if (action)
      return AstActionWrapper::Make(action);
    else
      return nullptr;
  }

  Token GetToken() { return nullptr; }

  bool CanBeWhatev() { return true; }

private:
  std::function<Action(Type left_in_type, Type right_in_type)> lambda;
};

#endif // TEREBINTH_AST_NODE_H_
