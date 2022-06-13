#ifndef TEREBINTH_NAMESPACE_H_
#define TEREBINTH_NAMESPACE_H_

#include "action.h"
#include "astnode.h"
#include "operator.h"
#include "token.h"
#include "type.h"


#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class StackFrame;

class NamespaceData;
typedef std::shared_ptr<NamespaceData> Namespace;

/**
 * @brief
 *
 */
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

private:
  /**
   * @brief Get the Nodes object
   *
   */
  void GetNodes();

  /**
   * @brief
   *
   */
  void NodesToMatchingActions();

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

#endif // TEREBINTH_NAMESPACE_H_