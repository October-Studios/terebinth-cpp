#include "../action.h"
#include "../error_handler.h"
#include "../cpp_program.h"
#include "../string_drawing.h"

class BranchAction: public ActionData {
public:
  BranchAction(Action left_input_in, Action action_in, Action right_input_in)
      : ActionData(action_in->GetReturnType(), Void, Void) {
    if (!action_in) {
      throw TerebinthError(std::string() + "branch action created sent null action", INTERNAL_ERROR);
    }

    if (!left_input_in) {
      throw TerebinthError(std::string() + "branch action created sent null left_input", INTERNAL_ERROR);
    }

    if (!right_input_in) {
      throw TerebinthError(std::string() + "branch action created sent null right_input", INTERNAL_ERROR);
    }

    action_ = action_in;
    left_input_ = left_input_in;
    right_input_ = right_input_in;

    if (!left_input_->GetInLeftType()->Matches(Void) || !left_input_->GetInRightType()->Matches(Void)) {
			throw TerebinthError(left_input_->GetDescription() +
          " put into branch even though its inputs are not void", INTERNAL_ERROR);
		}
		
		if (!right_input_->GetInLeftType()->Matches(Void) || !right_input_->GetInRightType()->Matches(Void)) {
			throw TerebinthError(right_input_->GetDescription() +
          " put into branch even though its inputs are not void", INTERNAL_ERROR);
		}
		
		if (!left_input_->GetReturnType()->Matches(action_->GetInLeftType())) {
			throw TerebinthError(left_input_->GetDescription() +
          " return type is not the same as the left input of " + action_->GetDescription(), INTERNAL_ERROR);
		}
		
		if (!right_input_->GetReturnType()->Matches(action_->GetInRightType())) {
			throw TerebinthError(right_input_->GetDescription() +
          " return type is not the same as the right input of " + action_->GetDescription(), INTERNAL_ERROR);
		}
  }

private:
  Action action_;
  Action left_input_;
  Action right_input_;
};
