#include "../action.h"
#include "../error_handler.h"
#include "../stack_frame.h"
#include "../ast_node.h"
#include "../string_num_conversion.h"
#include "../string_utils.h"

#include <cstring>

class FunctionAction: public ActionData {
public:
  FunctionAction(Action action_in, std::shared_ptr<StackFrame> stack_frame_in):
    ActionData(action_in->GetReturnType(), stack_frame_in->GetLeftInType(), stack_frame_in->GetRightInType()) {
      stack_frame_ = stack_frame_in;
      action_ = action_in;
      node_ = nullptr;

      SetDescription("function (" + GetInLeftType()->GetString() + "." + GetInRightType()->GetString()
          + " > " + GetReturnType()->GetString() + ")");

      if (action_->GetInLeftType() != Void || action_->GetInRightType() != Void) {
        throw TerebinthError(action_->GetDescription() + " put into function even though its inputs are not void",
            INTERNAL_ERROR);
      }
    }

private:
  std::shared_ptr<StackFrame> stack_frame_;
  Action action_ = nullptr;
  AstNode node_ = nullptr;
};

Action FunctionAction(Action action_in, std::shared_ptr<StackFrame> stack_frame_in) {
  return Action(new FunctionAction(action_in, stack_frame_in));
}

Action FunctionAction(AstNode node_in, Type return_type_in, std::shared_ptr<StackFrame> stack_frame_in) {
  return Action(new FunctionAction(std::move(node_in), return_type_in, stack_frame_in);
}
