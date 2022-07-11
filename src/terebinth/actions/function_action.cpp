#include "action.h"
#include "ast_node.h"
#include "error_handler.h"
#include "stack_frame.h"
#include "util/string_num_conversion.h"
#include "util/string_utils.h"

#include <cstring>

class FunctionAction : public ActionData {
public:
  FunctionAction(Action action_in, std::shared_ptr<StackFrame> stack_frame_in)
      : ActionData(action_in->GetReturnType(), stack_frame_in->GetLeftInType(),
                   stack_frame_in->GetRightInType()) {
    stack_frame_ = stack_frame_in;
    action_ = action_in;
    node_ = nullptr;

    SetDescription("function (" + GetInLeftType()->GetString() + "." +
                   GetInRightType()->GetString() + " > " +
                   GetReturnType()->GetString() + ")");

    if (action_->GetInLeftType() != Void || action_->GetInRightType() != Void) {
      throw TerebinthError(
          action_->GetDescription() +
              " put into function even though its inputs are not void",
          INTERNAL_ERROR);
    }
  }

  FunctionAction(AstNode node_in, Type return_type_in,
                 std::shared_ptr<StackFrame> stack_frame_in)
      : ActionData(return_type_in, stack_frame_in->GetLeftInType(),
                   stack_frame_in->GetRightInType()) {
    stack_frame_ = stack_frame_in;
    node_ = std::move(node_in);
    action_ = nullptr;

    SetDescription("function (" + GetInLeftType()->GetString() + "." +
                   GetInRightType()->GetString() + " > " +
                   GetReturnType()->GetString() + ")");
  }

  void ResolveAction() {
    if (!node_ || action_) {
      throw TerebinthError("FunctionAction::ResolveAction called when this "
                           "action is in the wrong state",
                           INTERNAL_ERROR);
    }

    action_ = node_->GetAction();

    if (!return_type_->IsVoid() &&
        !return_type_->Matches(action_->GetReturnType())) {
      throw TerebinthError("function body returns " +
                               action_->GetReturnType()->GetString() +
                               " instead of " + return_type_->GetString() +
                               "\n" + action_->GetDescription(),
                           SOURCE_ERROR);
    }

    if (!action_->GetInLeftType()->IsVoid() ||
        !action_->GetInRightType()->IsVoid()) {
      throw TerebinthError(
          action_->GetDescription() +
              " put into function even though its inputs are not void",
          INTERNAL_ERROR);
    }
  }

  std::string GetDescription() {
    return str::PutStringInTreeNodeBox("call func " + name_hint_);
  }

  bool IsFunction() { return true; }

  void *Execute(void *in_left, void *in_right) {
    if (!action_) {
      ResolveAction();
    }

    void *old_stack_ptr = stack_ptr_;

    stack_ptr_ = malloc(stack_frame_->GetSize());

    if (in_left) {
      memcpy((char *)stack_ptr_ + stack_frame_->GetLeftOffset(), in_left,
             GetInLeftType()->GetSize());
    }

    if (in_right) {
      memcpy((char *)stack_ptr_ + stack_frame_->GetRightOffset(), in_right,
             GetInRightType()->GetSize());
    }

    void *out = action_->Execute(nullptr, nullptr);

    free(stack_ptr_);
    stack_ptr_ = old_stack_ptr;

    return out;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    if (!action_) {
      ResolveAction();
    }

    std::string name = "%";

    name += name_hint_.empty() ? "func" : name_hint_;

    name += "_" + str::PtrToUniqueStr(&*action_);

    if (!prog->HasFunc(name)) {
      prog->PushFunc(name, GetInLeftType(), GetInRightType(), GetReturnType());
      if (GetReturnType()->IsCreatable() &&
          action_->GetReturnType() != GetReturnType()) {
        CppTupleCastAction(action_, GetReturnType())->AddToProg(prog);
      } else {
        action_->AddToProg(prog);
      }
      prog->Endln();
      prog->PopFunc();
    }

    prog->Name(name);

    prog->PushExpr();

    if (GetInLeftType()->IsCreatable()) {
      in_left->AddToProg(prog);
    }

    if (GetInRightType()->IsCreatable()) {
      if (GetInLeftType()->IsCreatable()) {
        prog->Code(", ");
      }

      in_right->AddToProg(prog);
    }

    prog->PopExpr();
  }

private:
  std::shared_ptr<StackFrame> stack_frame_;
  Action action_ = nullptr;
  AstNode node_ = nullptr;
};

Action FunctionAction(Action action_in,
                      std::shared_ptr<StackFrame> stack_frame_in) {
  return Action(FunctionAction(action_in, stack_frame_in));
}

Action FunctionAction(AstNode node_in, Type return_type_in,
                      std::shared_ptr<StackFrame> stack_frame_in) {
  return Action(
      FunctionAction(std::move(node_in), return_type_in, stack_frame_in));
}
