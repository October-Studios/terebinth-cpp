#include "../action.h"
#include "../error_handler.h"

class IfAction : public ActionData {
public:
  IfAction(Action condition_in, Action if_action_in) :
    ActionData(Void, Void, Void) {
    condition_ = condition_in;
    if_action_ = if_action_in;
  
    if (condition_->GetReturnType() != Bool) {
      error_.Log("IfAction created with condition action that does not return Bool", INTERNAL_ERROR);    
    }

    if (condition_->GetInLeftType() != Void || condition_->GetInRightType() != Void) {
      error_.Log("IfAction created with condition action that takes in something other than Void",
          INTERNAL_ERROR);
    }

    if (if_action_->GetInLeftType() != Void || if_action_->GetInRightType() != Void) {
      error_.Log("IfAction created with action that takes in something other than Void", INTERNAL_ERROR);
    }
  }

  std::string GetDescription() {
    return str::MakeRootUpBinaryTree("?", condition_->GetReturnType()->GetName(), "",
        condition_->GetDescription(), if_action_->GetDescription());
  }

  void* Execute(void* in_left, void* in_right) {
    void* condition_out = condition_->Execute(nullptr, nullptr);
    if (*((bool*)condition_out)) {
      free(if_action_->Execute(nullptr, nullptr));
    }
    free(condition_out);
    return nullptr;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram* prog) {
    prog->Code("if ");
    prog->PushExpr();
    condition_->AddToProg(void_action_, void_action_, prog);
    prog->PopExpr();
    prog->PushBlock();
    if_action_->AddToProg(void_action_, void_action_, prog);
    prog->Endln();
    prog->PopBlock();
  }

private:
  Action condition_;
  Action if_action_;
};

class IfElseAction : public ActionData {
public:
  IfElseAction(Action condition_in, Action if_action_in, Action else_action_in) :
    ActionData([&](){ return if_action_in->GetReturnType()->Matches(else_action_in->GetReturnType()) ?
        if_action_in->GetReturnType() : Void;}(), Void, Void) {
    return_val_ = GetReturnType() != Void;
    condition_ = condition_in;
    if_action_ = if_action_in;
    else_action_ else_action_in;

    if (condition->GetReturnType() != Bool) {
      error_.Log("IfElseAction created with condition action that does not return Bool", INTERNAL_ERROR);
    }

    if (condition_->GetInLeftType() != Void || condition_->GetInRightType() != Void ) {
      error_.Log("IfElseAction created with condition action that takes in something other than Void",
          INTERNAL_ERROR);
    }

    if (if_action_->GetInLeftType() != Void || if_action_->GetInRightType() != Void) {
      error_.Log("IfElseAction created with action that takes in something other than Void", INTERNAL_ERROR);
    }
  }

  std::string GetDescription() {
    std::string branch = str::MakeRootUpBinaryTree("╭┴╮\n| |", "fls", "tru", else_action_->GetDescription(),
        if_action_->GetDescription());
    return str::MakeRootUpBinaryTree("?", condition_->GetReturnType()->GetName(), "",
        condition_->GetDescription(), branch);
  }

  void* Execute(void* in_left, void* in_right) {
    void* out;
    void* condition_out = condition_->Execute(nullptr, nullptr);
    if (*((bool*)condition_out)) {
      out = if_action_->Execute(nullptr, nullptr);
    } else {
      out = else_action_->Execute(nullptr, nullptr);
    }
    free(condition_out);
    if (return_val_) {
      return out;
    } else {
      free(out);
      return nullptr;
    }
  }

  void AddToProg(Action in_left, Action in_right, CppProgram* prog) {
    if (return_val_ && prog->GetExprLevel() > 0) {
      prog->PushExpr();
        condition_->AddToProg(void_action_, void_action_, prog);
      prog->PopExpr();
      prog->Code(" ? ");
      prog->PushExpr();
        if_action_->AddToProg(void_action_, void_action_, prog);
      prog->PopExpr();
      prog->Code(" : ");
      prog->PushExpr();
        else_action_->AddToProg(void_action_, void_action_, prog);
      prog->PopExpr();
    } else {
      prog->Code("if ");
			prog->PushExpr();
				condition_->AddToProg(void_action_, void_action_, prog);
			prog->PopExpr();
			prog->PushBlock();
				if_action_->AddToProg(void_action_, void_action_, prog);
				prog->Endln();
			prog->PopBlock();
			prog->Code("else");
			prog->PushBlock();
				else_action_->AddToProg(void_action_, void_action_, prog);
				prog->Endln();
			prog->PopBlock();
    }
  }

private:
  Action condition_;
  Action if_action_;
  Action else_action_;
  bool return_val_ = false;
};

Action IfAction(Action condition_in, Action if_action_in) {
  return Action(IfAction(condition_in, if_action_in));
}

Action IfElseAction(Action condition_in, Action if_action_in, Action else_action_in) {
  return Action(IfElseAction(condition_in, if_action_in, else_action_in));
}
