#pragma once

#include <stdlib.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "cpp_program.h"
#include "stack_frame.h"
#include "type.h"
#include "util/string_drawing.h"

class ActionData;

extern std::shared_ptr<ActionData> void_action_;

class ActionData {
 public:
  ActionData(Type return_type_in, Type in_left_type_in, Type in_right_type_in);

  virtual ~ActionData() {}

  void SetDescription(std::string in) { description_ = in; }

  std::string ToString();
  std::string GetTypesString();
  virtual bool IsFunction() { return false; }

  Type &GetReturnType() { return return_type_; }
  Type &GetInLeftType() { return in_left_type_; }
  Type &GetInRightType() { return in_right_type_; }

  virtual std::string GetDescription() { return description_; }
  virtual void *Execute(void *in_left, void *in_right) = 0;
  virtual void AddToProg(std::shared_ptr<ActionData> in_left,
                         std::shared_ptr<ActionData> in_right,
                         CppProgram *prog) {
    prog->Comment("action '" + GetDescription() +
                  "' to cpp code not yet implemented");
  }
  void AddToProg(CppProgram *prog) {
    AddToProg(void_action_, void_action_, prog);
  }

  std::string name_hint_ = "";

 protected:
  Type return_type_;
  Type in_left_type_;
  Type in_right_type_;
  std::string description_;
};

using Action = std::shared_ptr<ActionData>;

class AstNodeBase;

Action LambdaActionT(
    Type in_left_type_in, Type in_right_type_in, Type return_type_in,
    std::function<void *(void *, void *)> lambda_in,
    std::function<void(Action in_left, Action in_right, CppProgram *prog)>
        cpp_writer_in,
    std::string text_in);
Action CreateNewVoidAction();

Action BranchActionT(Action left_input_in, Action action_in,
                     Action right_input_in);

Action FunctionActionT(Action action_in,
                       std::shared_ptr<StackFrame> stack_frame_in);
Action FunctionActionT(std::unique_ptr<AstNodeBase> node_in,
                       Type return_type_in,
                       std::shared_ptr<StackFrame> stack_frame_in);

Action AndActionT(Action first_action_in, Action second_action_in);
Action OrActionT(Action first_action_in, Action second_action_in);

Action IfActionT(Action condition_in, Action if_action_in);
Action IfElseActionT(Action condition_in, Action if_action_in,
                     Action else_action);

Action ListActionT(const std::vector<Action> &actions_in,
                   const std::vector<Action> &destroyers_in);

Action LoopActionT(Action condition_in, Action loop_action_in);
Action LoopActionT(Action condition_in, Action end_action_in,
                   Action loop_action_in);

Action MakeTupleActionT(const std::vector<Action> &source_actions_in);
Action GetElemFromTupleActionT(Type source, std::string name);
Action CppTupleCastActionT(Action action_in, Type returnType);

Action VarGetActionT(size_t in, Type type_in, std::string text_in);
Action VarSetActionT(size_t in, Type type_in, std::string text_in);
Action GlobalGetActionT(size_t in, Type type_in, std::string text_in);
Action GlobalSetActionT(size_t in, Type type_in, std::string text_in);

class NamespaceData;
Action ConstGetActionT(const void *in, Type type_in, std::string text_in,
                       std::shared_ptr<NamespaceData> ns);

Action TypeGetActionT(Type type_in);
