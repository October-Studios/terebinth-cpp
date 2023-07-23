export module Namespace;

import <memory>;
import <string>;
import <unordered_map>;
import <vector>;

import action;
import ast_node;
import Operator;
import token;
import type;

export
class StackFrame;

export
class NamespaceData;
typedef std::shared_ptr<NamespaceData> Namespace;

/**
 * @brief
 *
 */
export
class NamespaceData : public std::enable_shared_from_this<NamespaceData> {
 public:
  /**
   * @brief
   *
   * @return Namespace
   */
  static Namespace MakeRootNamespace();

  /**
   * @brief
   *
   * @return Namespace
   */
  Namespace MakeChild();

  /**
   * @brief
   *
   * @param name_in
   * @return Namespace
   */
  Namespace MakeChildAndFrame(std::string name_in);

  /**
   * @brief Get the String object
   *
   * @return std::string
   */
  std::string GetString();

  /**
   * @brief Get the String With Parents object
   *
   * @return std::string
   */
  std::string GetStringWithParents();

  /**
   * @brief Get the Stack Frame object
   *
   * @return std::shared_ptr<StackFrame>
   */
  std::shared_ptr<StackFrame> GetStackFrame() { return stack_frame_; }

  /**
   *
   *
   */
  void SetInput(Type left, Type right);
  void AddNode(AstNode node, std::string id);

  Type GetType(std::string name, bool throw_source_error,
               Token token_for_error);

  Action GetDestroyer(Type type);

  Action GetCopier(Type type);

  Action GetActionForTokenWithInput(Token token, Type left, Type right,
                                    bool dynamic, bool throw_source_error,
                                    Token token_for_error);

  std::vector<Action> *GetDestroyerActions() { return &destructor_actions_; }
  Action WrapInDestroyer(Action in);

 private:
  /**
   * @brief Get the Nodes object
   *
   */
  void GetNodes(std::vector<AstNodeBase *> &out, std::string text,
                bool check_actions, bool check_dynamic, bool check_whatev);

  /**
   * @brief
   *
   */
  void NodesToMatchingActions(std::vector<Action> &out,
                              std::vector<AstNodeBase *> &nodes,
                              Type left_in_type, Type right_in_type);

  /**
   * @brief
   *
   */
  class IdMap {
   public:
    /**
     * @brief
     *
     */
    void Add(std::string key, AstNode node);

    /**
     * @brief
     *
     */
    void Get(std::string key, std::vector<AstNodeBase *> &out);

   private:
    ///
    std::unordered_map<std::string, std::vector<AstNode>> nodes_;
  };

  /**
   * @brief
   *
   * @param type
   * @param name
   * @return Action
   */
  Action AddVar(Type type, std::string name);

  /**
   * @brief Construct a new Namespace Data object
   *
   * @param parent_in
   * @param stack_frame_in
   * @param name_in
   */
  NamespaceData(Namespace parent_in, std::shared_ptr<StackFrame> stack_frame_in,
                std::string name_in = "");

  ///
  std::string my_name_;
  ///
  std::shared_ptr<NamespaceData> parent_;
  ///
  std::shared_ptr<StackFrame> stack_frame_;
  ///
  IdMap actions_;
  ///
  IdMap dynamic_actions_;
  ///
  IdMap whatev_actions_;
  ///
  IdMap types_;
  ///
  std::vector<Action> destructor_actions_;
};
