module all_operators;

import error_handler;

AllOperators* ops_ = nullptr;

auto OpCreate(std::string text_in, int left_precedence_in,
              int right_precedence_in, bool overloadable_in) -> Operator;

void AllOperators::Init() { ops_ = new AllOperators(); }

AllOperators::AllOperators() {
#undef DECLARE_OP

#define DECLARE_OP(name, text, prece, input, overload) PutOpInMap(name);

  ALL_OPS;
}

void AllOperators::PutOpInMap(Operator op) { ops_map_[op->GetText()] = op; }

void AllOperators::Get(std::string text, std::vector<Operator>& out) {
  int start = 0;
  int end = text.size();

  while (start < int(text.size())) {
    while (true) {
      if (end <= start) {
        error_.Log("unknown operator '" + text + "'", SOURCE_ERROR);
      }

      auto i = ops_map_.find(text.substr(start, end - start));

      if (i == ops_map_.end()) {
        end--;
      } else {
        out.push_back(i->second);
        start = end;
        end = text.size();
        break;
      }
    }
  }
}

bool AllOperators::IsOpenBrac(Operator op) {
  return op == open_paren_ || op == open_sq_brac_ || op == open_cr_brac_;
}

bool AllOperators::IsCloseBrac(Operator op) {
  return op == close_paren_ || op == close_sq_brac_ || op == close_cr_brac_;
}
