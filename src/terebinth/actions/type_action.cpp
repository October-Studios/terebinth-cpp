#include "action.h"
#include "error_handler.h"

class TypeGetAction : public ActionData {
 public:
  explicit TypeGetAction(Type type_in)
      : ActionData(type_in->GetMeta(), Void, Void) {
    SetDescription(type_in->GetString() + " (type)");
  }

  auto Execute(void *in_left, void *in_right) -> void * {
    error_.Log("TypeGetAction::Execute called, which should not happen",
               RUNTIME_ERROR);
    return nullptr;
  }

  auto GetDescription() -> std::string {
    return str::PutStringInTreeNodeBox("{" + GetReturnType()->GetName() + "}");
  }
};

auto TypeGetAction(Type type_in) -> Action {
  return Action(TypeGetAction(type_in));
}
