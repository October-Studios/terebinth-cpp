#include "../action.h"
#include "../error_handler.h"

class ListAction : public ActionData {
public:
  ListAction(const std::vector<Action>& actions_in, const std::vector<Action>& destroyers_in) :
    ActionData(actions_in.size() > 0 ? actions_in.back()->GetReturnType() : Void), Void, Void) {
    actions_ = actions_in;
    destroyers_ = destroyers_in;

    for (auto i = actions_.begin(); i != actions_.end(); ++i) {
      if (!(*i)->GetInLeftType()->Matches(Void) || !(*i)->GetInRightType()->Matches(Void)) {
        error_.Log((*i)->GetDescription() + " put into action list even though its inputs are not void",
            INTERNAL_ERROR);
      }
    }
  }

  ~ListAction() {}

  std::string GetDescription() {
    std::vector<std::string> data;

    for (auto i = actions_.begin(); i != actions_.end(); ++i) {
      if (*i) {
        data.push_back((*i)->GetDescription());
      } else {
        data.push_back(str::PutStringInTreeNodeBox("[null action]"));
      }
    }

    return str::MakeList(data);
  }

  void* Execute(void* in_left, void* in_right) {
    auto i = actions_.begin();

    for (; i != std::prev(actions_.end()); ++i) {
      free((*i)->Execute(nullptr, nullptr));
    }

    void* return_val = (*i)->Execute(nullptr, nullptr);

    for (auto j : destroyers_) {
      free(j->Execute(nullptr, nullptr));
    }

    return return_val;
  }

  void AddToProg(Action in_left, Action in_right, CppProgram* prog) {
    AddToProg(prog, GetReturnType());
  }

  void AddToProg(CppProgram* prog, Type return_type) {
    bool should_return = (prog->GetBlockLevel() == 0 && prog->GetIfReturnsVal()) && !prog->IsMain();
  
    prog->PushBlock();

    for (auto i : actions_) {
      if (should_return && i == actions_.back()) {
        prog->DeclareVar("-out", return_type);
        prog->Name("-out");
        prog->Code(" = ");
        if (i->GetReturnType() != return_type) {
          CppTupleCastAction(i, return_type)->AddToProg(prog);
        } else {
          i->AddToProg(prog);
        }
      } else {
        i->AddToProg(prog);
      }
      prog->Endln();
    }
      
    for (auto i : destroyers_) {
      i->AddToProg(prog);
      prog->Endln();
    }

    if (should_return) {
      prog->Code("return ");
      prog->Name("-out");
      prog->Endln();
    }
    prog->PopBlock();
  }

private:
  std::vector<Action> actions_;
  std::vector<Action> destroyers_;
};

void AddListToProgWithCppCasting(ListAction* list, Type return_type, CppProgram* prog) {
  list->AddToProg(prog, return_type);
}

Action ListAction(const std::vector<Action>& actions_in, const std::vector<Action>& destroyers_in) {
  if (actions_in.size() == 0 && destroyers_in.size() == 0) {
    return void_action_;
  } else if (actions_in.size() == 1 && destroyers_in.size() == 0) {
    return actions_in[0];
  } else {
    return Action(ListAction(actions_in, destroyers_in));
  }
}
