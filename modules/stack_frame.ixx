module;
import <vector>;
export module stack_frame;

import type;
extern void *global_frame_ptr_;
extern void *stack_ptr_;

export
class StackFrame {
 public:
  void AddMember(Type in);
  void SetInput(Type left, Type right);

  size_t GetSize() { return frame_size_; }
  Type GetLeftInType();
  Type GetRightInType();
  size_t GetLeftOffset();
  size_t GetRightOffset();

 private:
  std::vector<Type> members_;
  size_t frame_size_ = 0;
  bool input_set_ = false;
  int left_input_index_ = -1;
  int right_input_index_ = -1;
  size_t left_input_offset_;
  size_t right_input_offset_;
};
