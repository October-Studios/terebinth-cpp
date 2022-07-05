#include "../action.h"
#include "../error_handler.h"

#include <vector>
#include <typeinfo>

class GetElemFromTupleAction;
class CppTupleCastAction;

class ListAction;
void AddListToProgWithCppCasting(ListAction* list, Type return_type, CppProgram* prog);

class MakeTupleAction : public ActionData {
public:
  MakeTupleAction(const std::vector<Action>& source_actions_in) :
    ActionData([&]() -> Type {
        TupleTypeMaker tuple;
        for (auto i = source_actions_in.begin(); i != source_actions_in.end(); ++i) {
          tuple.add((*i)->GetReturnType());
        }

        return tuple.Get(true);
    }(), Void, Void) {
    if (source_actions_in.size() <= 0) {
      error_.Log("MakeTupleAction created with empty list", INTERNAL_ERROR);
    }

    source_actions_ = source_actions_in;

    for (auto i = source_actions_.begin(); i != source_actions_.end(); ++i) {
      if (!(*i)->GetInLeftType()->Matches(Void) || !(*i)->GetInRightType()->Matches(Void)) {
        error_.Log((*i)->GetDescription() + " put into tuple creation even though its inputs are not void",
            INTERNAL_ERROR);
      }

      if ((*i)->GetReturnType()->Matches(Void)) {
        error_.Log((*i)->GetDescription() + " put into tuple creation even though its output is void",
            INTERNAL_ERROR);
      }
    }
  }

  std::string GetDescription() {
    return str::PutStringInTreeNodeBox("make tuple of type " + GetReturnType()->GetName());
  }

  void* Execute(void* in_left, void* in_right) {
    void* out = malloc(GetReturnType()->GetSize());
    size_t offset = 0;

    for (auto i = source_actions_.begin(); i != source_actions_.end(); ++i) {
      void* val = (*i)->Execute(nullptr, nullptr);
      memcpy((char*)out + offset, val, (*i)->GetReturnType()->GetSize());
      free(val);
      offset += (*i)->GetReturnType()->GetSize();
    }

    return out;
  }

  void AddToProg() {
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

  friend GetElemFromTupleAction;
  friend CppTupleCastAction;
};

class CppTupleCastAction : public ActionData {
public:
  CppTupleCastAction(Action action_in, Type return_type) :
    ActionData(return_type, Void, Void) {
    if ((action_in->GetReturnType()->GetType() != TypeBase::TUPLE && GetReturnType->GetType() !=
          TypeBase::TUPLE) || !action_in->GetReturnType()->Matches(GetReturnType())) {
      throw TerebinthError("CppCastAction was only designed to case matching tuples, which is not how it is being used",
          INTERNAL_ERROR);
    }

    action_ = action_in;
  }

  std::string GetDescription() {
    return "C++ cast";
  }

  void* Execute(void* in_left, void* in_right) {
    throw TerebinthError("CppCastAction was executed in the interpreter, which is not possible", INTERNAL_ERROR);
  }

  void AddToProg(Action in_left, Action in_right, CppProgram* prog) {
    if (GetReturnType()->GetAllSubTypes()->Size() == 1) {
      action_->AddToProg(prog);
    }	else if (typeid(*action) == typeid(MakeTupleAction)) {
			MakeTupleAction* real_action = (MakeTupleAction*)&*action_;
			
			prog->Code(prog->GetTypeCode(GetReturnType()));
			prog->PushExpr();
				for (auto i : real_action->source_actions_)
				{
					i->AddToProg(prog);
					if (i != real_action->source_actions_.back())
						prog->Code(", ");
				}
			prog->PopExpr();
		} else if (typeid(*action_) == typeid(*ListAction({void_action_, void_action_}, {}))) {
			AddListToProgWithCppCasting((ListAction*)&*action_, GetReturnType(), prog);
		} else if (GetReturnType()->GetType() != TypeBase::TUPLE) {
			action_->AddToProg(prog);
			prog->Code(".");
			prog->Code(action_->GetReturnType()->GetAllSubTypes()[0][0].name);
		}	else {
      std::string func_name = action_->GetReturnType()->GetCompactString() +
        "=>" + GetReturnType()->GetCompactString();
			
			if (!prog->HasFunc(func_name)) {
				Type arg_type = MakeTuple({{"in", action_->GetReturnType()}}, true);
				
				prog->PushFunc(func_name, "", Void, arg_type, GetReturnType());
					prog->DeclareVar("-out", GetReturnType());
					
					auto out_types = *GetReturnType()->GetAllSubTypes();
					
					if (action_->GetReturnType()->GetType() == TypeBase::TUPLE)
					{
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
				action->AddToProg(prog);
			prog->PopExpr();
		}
  }

private:
  Action action_;

  friend GetElemFromTupleAction;
};

class GetElemFromTupleAction : public ActionData {
public:
  GetElemFromTupleAction(Type type_in_in, std::string name_in) :
    ActionData(type_in_in->GetSubType(name_in).type, type_in_in, Void) {
    type_in = type_in_in;
    type_out_ = type_in_in->GetSubType(name_in).type;
    name_ = name_in;
    size_ = type_out_->GetSize();
    offset_ = type_in->GetSubType(name).offset;
  }

  std::string GetDescription() {
    return str::PutStringInTreeNodeBox(name);
  }

  void* Execute(void* in_left, void* in_right) {
    void* out = malloc(size);
    memcpy(out, (char*)in_left + offset_, size_);
    return out;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram* prog) {
    if (type_in->GetAllSubTypes()->Size() == 1) {
      in_left->AddToProg(prog);
      return;
    }

    MakeTupleAction* make_tuple_action = nullptr;

    if (typeid(*in_left) == typeid(MakeTupleAction)) {
      make_tuple_action = (MakeTupleAction*)&*in_left;
    } else if (typeid(*in_left) == typeid(CppTupleCastAction)) {
      Action cast_action = ((CppTupleCastAction*)&*in_left)->action_;

      if (typeid(*cast_action) == typeid(MakeTupleAction)) {
        make_tuple_action = (MakeTupleAction*)&*cast_action;
      }
    }

    if (make_tuple_action) {
      auto types = *in_left->GetReturnType()->GetAllSubTypes();

      for (int i = 0; i < int(types.size()); ++i) {
        if (types[i].name_ == name_) {
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

Action MakeTupleAction(const std::vector<Action>& source_actions_in) {
  return Action(MakeTupleAction(source_actions_in));
}

Action GetElemFromTupleAction(Type source, std::string name) {
  if (!source->GetSubType(name).type) {
    throw TerebinthError("could not find '" + name + "' in " + source->GetString(), SOURCE_ERROR);
  }

  Action out = Action(GetElemFromTupleAction(source, name));

  return out;
}

Action CppTupleCastAction(Action action_in, Type return_type) {
  return Action(CppTupleCastAction(action_in, return_type));
}
