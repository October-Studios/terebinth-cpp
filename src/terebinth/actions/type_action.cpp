#include "action.h"
#include "error_handler.h"

class TypeGetAction : public ActionData {
public:
  TypeGetAction(Type type_in) : ActionData(type_in->GetMeta(), Void, Void) {
    SetDescription(type_in->GetString() + " (type)");
  }

  void *Execute(void *in_left, void *in_right) {
    error_.Log("TypeGetAction::Execute called, which should not happen",
               RUNTIME_ERROR);
    return nullptr;
  }

  std::string GetDescription() {
    return str::PutStringInTreeNodeBox("{" + GetReturnType()->GetName() + "}");
  }
};

Action TypeGetAction(Type type_in) { return Action(TypeGetAction(type_in)); }
