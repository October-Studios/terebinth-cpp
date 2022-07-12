#pragma once

#include "cpp_program.h"
#include "stack_frame.h"
#include "util/string_drawing.h"
#include "type.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <stdlib.h>

class ActionData;

extern std::shared_ptr<ActionData> void_action_;

class ActionData {
public:
  ActionData(Type returnTypeIn, Type inLeftTypeIn, Type inRightTypeIn);

  virtual ~ActionData() {}

  void SetDescription(std::string in) { description_ = in; }

  std::string ToString();
  std::string GetTypesString();
  virtual bool IsFunction() { return false; }

  Type &GetReturnType() { return return_type_; }
  Type &GetInLeftType() { return in_left_type_; }
  Type &GetInRightType() { return in_right_type_; }

  virtual std::string GetDescription() { return description_; }
  virtual void *Execute(void *inLeft, void *inRight) = 0;
  virtual void AddToProg(std::shared_ptr<ActionData> inLeft,
                         std::shared_ptr<ActionData> inRight,
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

Action LambdaAction(
    Type in_left_type_in, Type in_right_type_in, Type return_type_in,
    std::function<void *(void *, void *)> lambda_in,
    std::function<void(Action in_left, Action in_right, CppProgram *prog)>
        cpp_writer_in,
    std::string text_in);
Action CreateNewVoidAction();

Action BranchAction(Action leftInputIn, Action actionIn, Action rightInputIn);

Action FunctionAction(Action actionIn, std::shared_ptr<StackFrame> stackFameIn);
Action FunctionAction(std::unique_ptr<AstNodeBase> nodeIn, Type returnTypeIn,
                      std::shared_ptr<StackFrame> stackFameIn);

Action AndAction(Action firstActionIn, Action secondActionIn);
Action OrAction(Action firstActionIn, Action secondActionIn);

Action IfAction(Action conditionIn, Action ifActionIn);
Action IfElseAction(Action conditionIn, Action ifActionIn, Action elseAction);

Action ListAction(const std::vector<Action> &actionsIn,
                  const std::vector<Action> &destroyersIn);

Action LoopAction(Action conditionIn, Action loopActionIn);
Action LoopAction(Action conditionIn, Action endActionIn, Action loopActionIn);

Action MakeTupleAction(const std::vector<Action> &sourceActionsIn);
Action GetElemFromTupleAction(Type source, std::string name);
Action CppTupleCastAction(Action actionIn, Type returnType);

Action VarGetAction(size_t in, Type type_in, std::string text_in);
Action VarSetAction(size_t in, Type type_in, std::string text_in);
Action GlobalGetAction(size_t in, Type type_in, std::string text_in);
Action GlobalSetAction(size_t in, Type type_in, std::string text_in);

class NamespaceData;
Action ConstGetAction(const void *in, Type type_in, std::string text_in,
                      std::shared_ptr<NamespaceData> ns);

Action TypeGetAction(Type type_in);
