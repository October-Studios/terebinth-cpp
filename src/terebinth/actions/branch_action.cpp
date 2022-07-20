#include "action.h"
#include "cpp_program.h"
#include "error_handler.h"
#include "util/string_drawing.h"

class BranchAction : public ActionData {
 public:
  BranchAction(Action left_input_in, Action action_in, Action right_input_in)
      : ActionData(action_in->GetReturnType(), Void, Void) {
    if (!action_in) {
      throw TerebinthError(
          std::string() + "branch action created sent null action",
          INTERNAL_ERROR);
    }

    if (!left_input_in) {
      throw TerebinthError(
          std::string() + "branch action created sent null left_input",
          INTERNAL_ERROR);
    }

    if (!right_input_in) {
      throw TerebinthError(
          std::string() + "branch action created sent null right_input",
          INTERNAL_ERROR);
    }

    action_ = action_in;
    left_input_ = left_input_in;
    right_input_ = right_input_in;

    if (!left_input_->GetInLeftType()->Matches(Void) ||
        !left_input_->GetInRightType()->Matches(Void)) {
      throw TerebinthError(
          left_input_->GetDescription() +
              " put into branch even though its inputs are not void",
          INTERNAL_ERROR);
    }

    if (!right_input_->GetInLeftType()->Matches(Void) ||
        !right_input_->GetInRightType()->Matches(Void)) {
      throw TerebinthError(
          right_input_->GetDescription() +
              " put into branch even though its inputs are not void",
          INTERNAL_ERROR);
    }

    if (!left_input_->GetReturnType()->Matches(action_->GetInLeftType())) {
      throw TerebinthError(
          left_input_->GetDescription() +
              " return type is not the same as the left input of " +
              action_->GetDescription(),
          INTERNAL_ERROR);
    }

    if (!right_input_->GetReturnType()->Matches(action_->GetInRightType())) {
      throw TerebinthError(
          right_input_->GetDescription() +
              " return type is not the same as the right input of " +
              action_->GetDescription(),
          INTERNAL_ERROR);
    }
  }

  auto GetDescription() -> std::string {
    if (left_input_ && action_ && right_input_) {
      return str::MakeRootUpBinaryTree(
          action_->GetDescription(), left_input_->GetReturnType()->GetName(),
          right_input_->GetReturnType()->GetName(),
          left_input_->GetDescription(), right_input_->GetDescription());
    } else {
      return "[branch with null element]";
    }
  }

  auto Execute(void *in_left, void *in_right) -> void * {
    void *left_data = left_input_->Execute(nullptr, nullptr);
    void *right_data = right_input_->Execute(nullptr, nullptr);
    void *out_data = action_->Execute(left_data, right_data);
    free(left_data);
    free(right_data);
    return out_data;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    Action left_in_tmp = left_input_;
    Action right_in_tmp = right_input_;

    if (left_in_tmp->GetReturnType() != action_->GetInLeftType()) {
      left_in_tmp = CppTupleCastActionT(left_in_tmp, action_->GetInLeftType());
    }

    if (right_in_tmp->GetReturnType() != action_->GetInRightType()) {
      right_in_tmp =
          CppTupleCastActionT(right_in_tmp, action_->GetInRightType());
    }

    action_->AddToProg(left_in_tmp, right_in_tmp, prog);
  }

 private:
  Action action_;
  Action left_input_;
  Action right_input_;
};

class RightBranchAction : public ActionData {
 public:
  RightBranchAction(Action action_in, Action right_input_in)
      : ActionData(action_in->GetReturnType(), Void, Void) {
    if (!action_in)
      throw TerebinthError(
          std::string() + "branch action created sent null action",
          INTERNAL_ERROR);

    if (!right_input_in)
      throw TerebinthError(
          std::string() + "branch action created sent null right_input",
          INTERNAL_ERROR);

    action_ = action_in;
    right_input_ = right_input_in;

    if (!right_input_->GetInLeftType()->Matches(Void) ||
        !right_input_->GetInRightType()->Matches(Void)) {
      throw TerebinthError(
          right_input_->GetDescription() +
              " put into branch even though its inputs are not void",
          INTERNAL_ERROR);
    }

    if (!right_input_->GetReturnType()->Matches(action_->GetInRightType())) {
      throw TerebinthError(
          right_input_->GetDescription() +
              " return type is not the same as the right input of " +
              action_->GetDescription(),
          INTERNAL_ERROR);
    }
  }

  ~RightBranchAction() = default;

  auto GetDescription() -> std::string {
    if (action_ && right_input_) {
      return str::MakeRootUpBinaryTree(action_->GetDescription(), "",
                                       right_input_->GetReturnType()->GetName(),
                                       "", right_input_->GetDescription());
    } else {
      return "[branch with null element]";
    }
  }

  auto Execute(void *in_left, void *in_right) -> void * {
    void *right_data = right_input_->Execute(nullptr, nullptr);
    void *out_data = action_->Execute(nullptr, right_data);
    free(right_data);
    return out_data;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    Action right_in_tmp = right_input_;

    if (right_in_tmp->GetReturnType() != action_->GetInRightType())
      right_in_tmp =
          CppTupleCastActionT(right_in_tmp, action_->GetInRightType());

    action_->AddToProg(void_action_, right_in_tmp, prog);
  }

 private:
  Action action_ = nullptr;
  Action right_input_ = nullptr;
};

class LeftBranchAction : public ActionData {
 public:
  LeftBranchAction(Action left_input_in, Action action_in)
      : ActionData(action_in->GetReturnType(), Void, Void) {
    if (!action_in)
      throw TerebinthError(
          std::string() + "branch action created sent null action",
          INTERNAL_ERROR);

    if (!left_input_in)
      throw TerebinthError(
          std::string() + "branch action created sent null leftInput",
          INTERNAL_ERROR);

    action_ = action_in;
    left_input_ = left_input_in;

    if (!left_input_->GetInLeftType()->Matches(Void) ||
        !left_input_->GetInRightType()->Matches(Void)) {
      throw TerebinthError(
          left_input_->GetDescription() +
              " put into branch even though its inputs are not void",
          INTERNAL_ERROR);
    }

    if (!left_input_->GetReturnType()->Matches(action_->GetInLeftType())) {
      throw TerebinthError(
          left_input_->GetDescription() +
              " return type is not the same as the left input of " +
              action_->GetDescription(),
          INTERNAL_ERROR);
    }
  }

  auto GetDescription() -> std::string {
    if (left_input_ && action_) {
      return str::MakeRootUpBinaryTree(action_->GetDescription(),
                                       left_input_->GetReturnType()->GetName(),
                                       "", left_input_->GetDescription(), "");
    } else {
      return "[branch with null element]";
    }
  }

  auto Execute(void *in_left, void *in_right) -> void * {
    void *left_data = left_input_->Execute(nullptr, nullptr);
    void *out_data = action_->Execute(left_data, nullptr);
    free(left_data);
    return out_data;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    Action left_in_tmp = left_input_;

    if (left_in_tmp->GetReturnType() != action_->GetInLeftType())
      left_in_tmp = CppTupleCastActionT(left_in_tmp, action_->GetInLeftType());

    action_->AddToProg(left_in_tmp, void_action_, prog);
  }

 private:
  Action left_input_;
  Action action_;
};

auto BranchActionT(Action left_input_in, Action action_in,
                   Action right_input_in) -> Action {
  if (left_input_in->GetReturnType()->IsVoid()) {
    if (right_input_in->GetReturnType()->IsVoid()) {
      return action_in;
    } else {
      return Action(new RightBranchAction(action_in, right_input_in));
    }
  } else {
    if (right_input_in->GetReturnType()->IsVoid()) {
      return Action(new LeftBranchAction(left_input_in, action_in));
    } else {
      return Action(new BranchAction(left_input_in, action_in, right_input_in));
    }
  }
}
