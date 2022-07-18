#include "stack_frame.h"
#include "error_handler.h"
#include "util/string_utils.h"

void *global_frame_ptr_ = nullptr;
void *stack_ptr_ = nullptr;

void StackFrame::AddMember(Type in) {
  if (!in->IsCreatable()) {
    throw TerebinthError("tried to insert uncreatable type " + in->GetString() +
                             " into stack frame",
                         INTERNAL_ERROR);
  }

  members_.push_back(in);
  frame_size_ += in->GetSize();
}

void StackFrame::SetInput(Type left, Type right) {
  if (input_set_) {
    error_.Log("StackFrame::SetInput called multiple times", INTERNAL_ERROR);
  } else {
    if (left->IsCreatable()) {
      left_input_offset_ = frame_size_;
      left_input_index_ = members_.size();
      AddMember(left);
    } else if (left != Void) {
      throw TerebinthError("stack frame left input set to " +
                               left->GetString() +
                               " which is neither creatable nor a normal void",
                           INTERNAL_ERROR);
    }

    if (right->IsCreatable()) {
      right_input_offset_ = frame_size_;
      right_input_index_ = members_.size();
      AddMember(right);
    } else if (right != Void) {
      throw TerebinthError("stack frame right input set to " +
                               right->GetString() +
                               " which is neither creatable nor a normal void",
                           INTERNAL_ERROR);
    }

    input_set_ = true;
  }
}

Type StackFrame::GetLeftInType() {
  if (left_input_index_ >= 0) {
    return members_[left_input_index_];
  } else {
    return Void;
  }
}

Type StackFrame::GetRightInType() {
  if (right_input_index_ >= 0) {
    return members_[right_input_index_];
  } else {
    return Void;
  }
}

size_t StackFrame::GetLeftOffset() {
  if (left_input_offset_ >= 0) {
    return left_input_offset_;
  } else {
    error_.Log("tried to get the left input offset from a stack frame that "
               "does not have a left input",
               INTERNAL_ERROR);
    return 0;
  }
}

size_t StackFrame::GetRightOffset() {
  if (right_input_offset_ >= 0) {
    return right_input_offset_;
  } else {
    error_.Log("tried to get the right input offset from a stack frame that "
               "does not have a right input",
               INTERNAL_ERROR);
    return 0;
  }
}
