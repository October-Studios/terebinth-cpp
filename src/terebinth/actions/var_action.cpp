#include "action.h"
#include "cpp_program.h"
#include "error_handler.h"
#include "namespace.h"
#include "stack_frame.h"
#include "util/string_drawing.h"
#include "util/string_num_conversion.h"

class VarGetAction : public ActionData {
public:
  VarGetAction(size_t in, void **stack_ptr_ptr_in, Type type_in,
               std::string id_in)
      : ActionData(type_in, Void, Void) {
    offset_ = in;
    stack_ptr_ptr_ = stack_ptr_ptr_in;
    name_hint_ = id_in;

    SetDescription("get " + type_in->GetString() + " '" + id_in + "'");
  }

  void *Execute(void *in_left, void *in_right) {
    if (!(*stack_ptr_ptr_)) {
      throw TerebinthError(
          "VarGetAction::Executed called while stack pointer is still null",
          RUNTIME_ERROR);
    }

    void *out = malloc(return_type_->GetSize());
    memcpy(out, (char *)(*stack_ptr_ptr_) + offset_, return_type_->GetSize());
    return out;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    prog->DeclareVar(name_hint_, GetReturnType());
    prog->Name(name_hint_);
  }

  std::string GetDescription() {
    return str::PutStringInTreeNodeBox("get " + name_hint_);
  }

private:
  void **stack_ptr_ptr_;
  size_t offset_;
};

class VarSetAction : public ActionData {
public:
  VarSetAction(size_t in, void **stack_ptr_ptr_in, Type type_in,
               std::string id_in)
      : ActionData(Void, Void, type_in) {
    offset_ = in;
    stack_ptr_ptr_ = stack_ptr_ptr_in;
    name_hint_ = id_in;

    SetDescription("set " + type_in->GetString() + " '" + id_in + "'");
  }

  void *Execute(void *left, void *right) {
    if (!(*stack_ptr_ptr_)) {
      throw TerebinthError(
          "VarSetAction::Execute called while stack pointer is still null",
          RUNTIME_ERROR);
    }

    memcpy((char *)(*stack_ptr_ptr_) + offset_, right,
           in_right_type_->GetSize());
    void *out = malloc(return_type_->GetSize());
    memcpy(out, (char *)(*stack_ptr_ptr_) + offset_, in_right_type_->GetSize());

    return nullptr;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    prog->DeclareVar(name_hint_, GetInRightType());
    prog->Name(name_hint_);
    prog->Code(" = ");
    prog->PushExpr();
    in_right->AddToProg(prog);
    prog->PopExpr();
  }

  std::string GetDescription() {
    return str::PutStringInTreeNodeBox("set " + name_hint_);
  }

private:
  void **stack_ptr_ptr_;
  size_t offset_;
};

class ConstGetAction : public ActionData {
public:
  ConstGetAction(const void *in, Type type_in, std::string text_in)
      : ActionData(type_in, Void, Void) {
    data_ = malloc(return_type_->GetSize());
    memcpy(data_, in, return_type_->GetSize());

    SetDescription(text_in);
  }

  ~ConstGetAction() { free(data_); }

  void *Execute(void *in_left, void *in_right) {
    void *out = malloc(return_type_->GetSize());
    memcpy(out, data_, return_type_->GetSize());
    return out;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    if (GetReturnType() == String) {
      prog->Code(prog->GetTypeCode(String));
      prog->PushExpr();

      auto size_info = GetReturnType()->GetSubType("_size");
      auto data_info = GetReturnType()->GetSubType("_data");
      if (size_info.type != Int || data_info.type != Byte->GetPtr()) {
        throw TerebinthError(
            "ConstGetAction::AddToProg failed to access String properties",
            INTERNAL_ERROR);
      }

      prog->Code(Int->GetCppLiteral((char *)data_ + size_info.offset, prog));
      prog->Code(", ");
      prog->Code("(unsigned char*)\"");
      int len = *(int *)((char *)data_ + size_info.offset);
      for (int i = 0; i < len; ++i) {
        char c = *((*(char **)((char *)data_ + data_info.offset)) + i);

        if (c == '"') {
          prog->Code("\\\"");
        } else if (c == '\\') {
          prog->Code("\\\\");
        } else if (c >= 32 && c <= 126) {
          prog->Code(std::string() + c);
        } else if (c == '\n') {
          prog->Code("\\n");
        } else {
          prog->Code(str::CharToCppStringLiteralEscaped(c));
        }
      }

      prog->Code("\"");

      prog->PopExpr();
    } else {
      prog->Code(GetReturnType()->GetCppLiteral(data_, prog));
    }
  }

  std::string GetDescription() {
    return str::PutStringInTreeNodeBox(description_);
  }

private:
  void *data_;
};

Action VarGetActionT(size_t in, Type type_in, std::string text_in) {
  return Action(new VarGetAction(in, &stack_ptr_, type_in, text_in));
}

Action VarSetActionT(size_t in, Type type_in, std::string text_in) {
  return Action(new VarSetAction(in, &stack_ptr_, type_in, text_in));
}

Action GlobalGetActionT(size_t in, Type type_in, std::string text_in) {
  return Action(new VarGetAction(in, &global_frame_ptr_, type_in, text_in));
}

Action GlobalSetActionT(size_t in, Type type_in, std::string text_in) {
  return Action(new VarSetAction(in, &global_frame_ptr_, type_in, text_in));
}

Action ConstGetActionT(const void *in, Type type_in, std::string text_in,
                      Namespace ns) {
  Action action = Action(new ConstGetAction(in, type_in, text_in));
  if (ns) {
    Action copier = ns->GetCopier(type_in);
    if (copier) {
      action = BranchActionT(void_action_, copier, action);
    }
  }
  return action;
}
