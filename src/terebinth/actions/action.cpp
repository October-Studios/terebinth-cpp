#include "action.h"
#include "error_handler.h"

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
