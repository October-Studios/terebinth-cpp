#include "../action.h"
#include "../error_handler.h"
#include "../string_utils.h"

ActionData::ActionData(Type return_type_in, Type in_left_type_in,
                       Type in_right_type_in) {
  return_type_ = return_type_in;
  in_left_type_ = in_left_type_in;
  in_right_type_ = in_right_type_in;

  if (!return_type_ || !in_left_type_ || !in_right_type_) {
    throw TerebinthError("ActionData created with null type", INTERNAL_ERROR);
  }
}

std::string ActionData::ToString() { return description_; }

std::string ActionData::GetTypesString() {
  return return_type_->GetString() + " <- " + in_left_type_->GetString() + "." + in_right_type_->GetString();
}

class VoidAction: public ActionData {
public:
  VoidAction(): ActionData(Void, Void, Void) {
    SetDescription("Void Action");
  }

  void* Execute(void* in_left, void* in_right) {
    return nullptr;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram* prog) {
    if (prog->GetExprLevel() > 0) {
      prog->Comment("void");
    }
  }

  std::string GetDescription() {
    return str::PutStringInTreeNodeBox("void");
  }
};

class LambdaAction: public ActionData {
public:
  LambdaAction(Type in_left_type_in, Type in_right_type_in, Type return_type_in,
      std::function<void*(void*, void*)> lambda_in,
      std::function<void(Action in_left, Action in_right, CppProgram* prog)> cpp_writer_in,
      std::string text_in) : ActionData(return_type_in, in_left_type_in, in_right_type_in) {
    if (cpp_writer_in == nullptr) {
      cpp_writer_ = [=](Action in_left, Action in_right, CppProgram* prog) {
        prog->Comment("lambda action '" + text_in + "' has not yet been implemented for C++");
      };
    } else {
      cpp_writer_ = cpp_writer_in;
    }

    if (lambda_in == nullptr) {
      lambda_ = [=](void* in_left, void* in_right)->void* {
        throw TerebinthError("action '" + text_in + "' has not yet been implemented for the interpreter", INTERNAL_ERROR);
      };
    } else {
      lambda_ = lambda_in;
    }

    SetDescription(text_in);
  }

  void* Execute(void* in_left, void* in_right) {
    return lambda_(in_left, in_right);
  }

  void AddToProg(Action in_left, Action in_right, CppProgram* prog) {
    cpp_writer_(in_left, in_right, prog);
  }

  std::string GetDescription() {
    return str::PutStringInTreeNodeBox(description_);
  }

private:
  std::function<void*(void*, void*)> lambda_;
  std::function<void(Action in_left, Action in_right, CppProgram* prog)> cpp_writer_;
  std::string cpp_code_;
};

Action LambdaAction(Type in_left_type_in, Type in_right_type_in, Type return_type_in,
    std::function<void*(void*, void*)> lambda_in,
    std::function<void(Action in_left, Action in_right, CppProgram* prog)> cpp_writer_in,
    std::string text_in) {
  return Action(LambdaAction(in_left_type_in, in_right_type_in, return_type_in, lambda_in, cpp_writer_in, text_in));
}

Action CreateNewVoidAction() {
  return Action(new VoidAction());
}
