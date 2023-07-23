module;
import <typeinfo>;
import <vector>;

import action;
import error_handler;

class GetElemFromTupleAction;
class CppTupleCastAction;

class ListAction;
void AddListToProgWithCppCasting(ListAction *list, Type return_type,
                                 CppProgram *prog);

class MakeTupleAction : public ActionData {
 public:
  explicit MakeTupleAction(const std::vector<Action> &source_actions_in)
      : ActionData(
            [&]() -> Type {
              TupleTypeMaker tuple;
              for (auto i = source_actions_in.begin();
                   i != source_actions_in.end(); ++i) {
                tuple.Add((*i)->GetReturnType());
              }

              return tuple.Get(true);
            }(),
            Void, Void) {
    if (source_actions_in.size() <= 0) {
      error_.Log("MakeTupleAction created with empty list", INTERNAL_ERROR);
    }

    source_actions_ = source_actions_in;

    for (auto i = source_actions_.begin(); i != source_actions_.end(); ++i) {
      if (!(*i)->GetInLeftType()->Matches(Void) ||
          !(*i)->GetInRightType()->Matches(Void)) {
        error_.Log(
            (*i)->GetDescription() +
                " put into tuple creation even though its inputs are not void",
            INTERNAL_ERROR);
      }

      if ((*i)->GetReturnType()->Matches(Void)) {
        error_.Log(
            (*i)->GetDescription() +
                " put into tuple creation even though its output is void",
            INTERNAL_ERROR);
      }
    }
  }

  auto GetDescription() -> std::string {
    return str::PutStringInTreeNodeBox("make tuple of type " +
                                       GetReturnType()->GetName());
  }

  auto Execute(void *in_left, void *in_right) -> void * {
    void *out = malloc(GetReturnType()->GetSize());
    size_t offset = 0;

    for (auto i = source_actions_.begin(); i != source_actions_.end(); ++i) {
      void *val = (*i)->Execute(nullptr, nullptr);
      memcpy((char *)out + offset, val, (*i)->GetReturnType()->GetSize());
      free(val);
      offset += (*i)->GetReturnType()->GetSize();
    }

    return out;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    if (source_actions_.size() == 1) {
      source_actions_[0]->AddToProg(prog);
      return;
    }

    prog->Code(prog->GetTypeCode(GetReturnType()));
    prog->PushExpr();
    bool start = true;
    for (auto i : source_actions_) {
      if (!start) prog->Code(", ");
      start = false;

      i->AddToProg(prog);
    }
    prog->PopExpr();
  }

 private:
  std::vector<Action> source_actions_;

  friend class GetElemFromTupleAction;
  friend class CppTupleCastAction;
};

class CppTupleCastAction : public ActionData {
 public:
  CppTupleCastAction(Action action_in, Type return_type)
      : ActionData(return_type, Void, Void) {
    if ((action_in->GetReturnType()->GetType() != TypeBase::TUPLE &&
         GetReturnType()->GetType() != TypeBase::TUPLE) ||
        !action_in->GetReturnType()->Matches(GetReturnType())) {
      throw TerebinthError(
          "CppCastAction was only designed to case matching "
          "tuples, which is not how it is being used",
          INTERNAL_ERROR);
    }

    action_ = action_in;
  }

  auto GetDescription() -> std::string { return "C++ cast"; }

  auto Execute(void *in_left, void *in_right) -> void * {
    throw TerebinthError(
        "CppCastAction was executed in the interpreter, which is not possible",
        INTERNAL_ERROR);
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    if (GetReturnType()->GetAllSubTypes()->size() == 1) {
      action_->AddToProg(prog);
    } else if (typeid(*action_) == typeid(MakeTupleAction)) {
      MakeTupleAction *real_action = (MakeTupleAction *)&*action_;

      prog->Code(prog->GetTypeCode(GetReturnType()));
      prog->PushExpr();
      for (auto i : real_action->source_actions_) {
        i->AddToProg(prog);
        if (i != real_action->source_actions_.back()) prog->Code(", ");
      }
      prog->PopExpr();
    } else if (typeid(*action_) ==
               typeid(*ListActionT({void_action_, void_action_}, {}))) {
      AddListToProgWithCppCasting((ListAction *)&*action_, GetReturnType(),
                                  prog);
    } else if (GetReturnType()->GetType() != TypeBase::TUPLE) {
      action_->AddToProg(prog);
      prog->Code(".");
      prog->Code(action_->GetReturnType()->GetAllSubTypes()[0][0].name);
    } else {
      std::string func_name = action_->GetReturnType()->GetCompactString() +
                              "=>" + GetReturnType()->GetCompactString();

      if (!prog->HasFunc(func_name)) {
        Type arg_type = MakeTuple({{"in", action_->GetReturnType()}}, true);

        prog->PushFunc(func_name, "", Void, arg_type, GetReturnType());
        prog->DeclareVar("-out", GetReturnType());

        auto out_types = *GetReturnType()->GetAllSubTypes();

        if (action_->GetReturnType()->GetType() == TypeBase::TUPLE) {
          auto in_types = *action_->GetReturnType()->GetAllSubTypes();

          for (int i = 0; i < int(in_types.size()); ++i) {
            {
              prog->Name("-out");
              prog->Code(".");
              prog->Code(out_types[i].name);
              prog->Code(" = ");
              prog->Name("in");
              prog->Code(".");
              prog->Code(in_types[i].name);
              prog->Endln();
            }
          }
        } else {
          prog->Name("-out");
          prog->Code(".");
          prog->Code(out_types[0].name);
          prog->Code(" = ");
          prog->Name("in");
          prog->Endln();
        }

        prog->Code("return ");
        prog->Name("-out");
        prog->Endln();

        prog->PopFunc();
      }

      prog->Name(func_name);
      prog->PushExpr();
      action_->AddToProg(prog);
      prog->PopExpr();
    }
  }

 private:
  Action action_;

  friend class GetElemFromTupleAction;
};

class GetElemFromTupleAction : public ActionData {
 public:
  GetElemFromTupleAction(Type type_in_in, std::string name_in)
      : ActionData(type_in_in->GetSubType(name_in).type, type_in_in, Void) {
    type_in_ = type_in_in;
    type_out_ = type_in_in->GetSubType(name_in).type;
    name_ = name_in;
    size_ = type_out_->GetSize();
    offset_ = type_in_->GetSubType(name_).offset;
  }

  auto GetDescription() -> std::string {
    return str::PutStringInTreeNodeBox(name_);
  }

  auto Execute(void *in_left, void *in_right) -> void * {
    void *out = malloc(size_);
    memcpy(out, static_cast<char *>(in_left) + offset_, size_);
    return out;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram *prog) {
    if (type_in_->GetAllSubTypes()->size() == 1) {
      in_left->AddToProg(prog);
      return;
    }

    MakeTupleAction *make_tuple_action = nullptr;

    if (typeid(*in_left) == typeid(MakeTupleAction)) {
      make_tuple_action = (MakeTupleAction *)&*in_left;
    } else if (typeid(*in_left) == typeid(CppTupleCastAction)) {
      Action cast_action = ((CppTupleCastAction *)&*in_left)->action_;

      if (typeid(*cast_action) == typeid(MakeTupleAction)) {
        make_tuple_action = (MakeTupleAction *)&*cast_action;
      }
    }

    if (make_tuple_action) {
      auto types = *in_left->GetReturnType()->GetAllSubTypes();

      for (int i = 0; i < int(types.size()); ++i) {
        if (types[i].name == name_) {
          make_tuple_action->source_actions_[i]->AddToProg(prog);
          break;
        }
      }
    } else {
      in_left->AddToProg(prog);
      prog->Code("." + name_);
    }
  }

 private:
  Type type_in_;
  Type type_out_;
  int offset_;
  size_t size_;
  std::string name_;
};

auto MakeTupleActionT(const std::vector<Action> &source_actions_in) -> Action {
  return Action(new MakeTupleAction(source_actions_in));
}

auto GetElemFromTupleActionT(Type source, std::string name) -> Action {
  if (!source->GetSubType(name).type) {
    throw TerebinthError(
        "could not find '" + name + "' in " + source->GetString(),
        SOURCE_ERROR);
  }

  Action out = Action(new GetElemFromTupleAction(source, name));

  return out;
}

auto CppTupleCastActionT(Action action_in, Type return_type) -> Action {
  return Action(new CppTupleCastAction(action_in, return_type));
}
