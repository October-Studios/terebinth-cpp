#include "../action.h"
#include "../error_handler.h"

class LoopAction : public ActionData {
public:
  LoopAction(Action condition_in, Action end_action_in, Action loop_action_in)
      : ActionData(Void, Void, Void) {
    condition_ = condition_in;
    loop_action_ = loop_action_in;
    end_action_ = end_action_in;

    if (condition_->GetReturnType() != Bool) {
      error_.Log(
          "LoopAction created with condition action that does not return Bool",
          INTERNAL_ERROR);
    }

    if (condition_->GetInLeftType() != Void ||
        condition_->GetInRightType() != Void) {
      error_.Log("LoopAction created with condition action that takes in "
                 "something other than Void",
                 INTERNAL_ERROR);
    }

    if (loop_action_->GetInLeftType() != Void ||
        loop_action_->GetInRightType() != Void) {
      error_.Log("LoopAction created with action that takes in something other "
                 "than Void",
                 INTERNAL_ERROR);
    }
  }

  std::string GetDescription() {
    std::string body = loop_action_->GetDescription();
    if (end_action_ != void_action_) {
      std::vector<std::string> data = {body, end_action_->GetDescription()};
      body = str::MakeList(data);
    }

    return str::MakeRootUpBinaryTree("@",
                                     condition_->GetReturnType()->GetName(), "",
                                     condition_->GetDescription(), body);
  }

  void *Execute(void *in_left, void *in_right) {
    void *condition_out;

    while (true) {
      condition_out = condition_->Execute(nullptr, nullptr);
      if (!(*((bool *)condition_out))) {
        break;
      }
      free(condition_out);
      free(loop_action_->Execute(nullptr, nullptr));
      free(end_action_->Execute(nullptr, nullptr));
    }

    free(condition_out);

    return nullptr;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    prog->Code("while ");

    prog->PushExpr();
    condition_->AddToProg(prog);
    prog->PopExpr();

    prog->PushBlock();
    loop_action_->AddToProg(prog);
    prog->Endln();
    if (end_action_ != void_action_) {
      end_action_->AddToProg(prog);
      prog->Endln();
    }
    prog->PopBlock();
  }

private:
  Action condition_;
  Action loop_action_;
  Action end_action_;
};

Action LoopAction(Action condition_in, Action loop_action_in) {
  return Action(LoopAction(condition_in, void_action_, loop_action_in));
}

Action LoopAction(Action condition_in, Action end_action_in,
                  Action loop_action_in) {
  return Action(LoopAction(condition_in, end_action_in, loop_action_in));
}
