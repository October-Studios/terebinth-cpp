#ifndef TEREBINTH_ACTION_H_
#define TEREBINTH_ACTION_H_

#include "cpp_program.h"
#include "stack_frame.h"
#include "type.h"
#include "utils/string_drawing.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <stdlib.h>

class ActionData;

extern std::shared_ptr<ActionData> voidAction;

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
  void AddToProg(CppProgram *prog) { AddToProg(voidAction, voidAction, prog); }

  std::string name_hint_ = "";

protected:
  Type return_type_;
  Type in_left_type_;
  Type in_right_type_;
  std::string description_;
};

typedef std::shared_ptr<ActionData> Action;

class AstNodeBase;

// Action lambdaAction(Type returnTypeIn, std::function<void*(void*,void*)>
// lambdaIn, Type inLeftTypeIn, Type inRightTypeIn, std::string textIn);
Action LambdaAction(
    Type inLeftTypeIn, Type inRightTypeIn, Type returnTypeIn,
    std::function<void *(void *, void *)> lambdaIn,
    std::function<void(Action inLeft, Action inRight, CppProgram *prog)>
        addCppToProg,
    std::string textIn);
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
// Action getElemFromTupleAction(Action source, std::string name);
Action GetElemFromTupleAction(Type source, std::string name);
Action CppTupleCastAction(Action actionIn, Type returnType);

Action VarGetAction(size_t in, Type typeIn, std::string idIn);
Action VarSetAction(size_t in, Type typeIn, std::string idIn);
Action GlobalGetAction(size_t in, Type typeIn, std::string idIn);
Action GlobalSetAction(size_t in, Type typeIn, std::string idIn);

class NamespaceData;
Action ConstGetAction(const void *in, Type typeIn, std::string idIn,
                      std::shared_ptr<NamespaceData> ns);

Action TypeGetAction(Type typeIn);

#endif // TEREBINTH_ACTION_H_