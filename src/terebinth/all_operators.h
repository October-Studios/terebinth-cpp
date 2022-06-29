#ifndef TEREBINTH_ALL_OPERATORS_H_
#define TEREBINTH_ALL_OPERATORS_H_

#include "operator.h"
#include <unordered_map>
#include <vector>

class AllOperators {
public:
  static void Init();

  void Get(std::string text, std::vector<Operator> &out);

#define DECLARE_OP(name, text, prece, input, overload)                         \
  const Operator name{new OperatorData(text, prece, input, overload)};

#define ALL_OPS
DECLARE_OP(loop_, "@", 5, OperatorData::BOTH, false);	\
DECLARE_OP(ifOp,		"?", 6, OperatorData::BOTH,		false	);	\
DECLARE_OP(pipe,		"|", 6, OperatorData::BOTH,		false	);	\
DECLARE_OP(colon,		":", 24, OperatorData::BOTH,		false	);	\
DECLARE_OP(doubleColon, "::", 24, OperatorData::BOTH,		false	);	\
DECLARE_OP(comma,		",", 35, OperatorData::BOTH,		false	);	\
DECLARE_OP(orOp,		"||", 36, OperatorData::BOTH,		false	);	\
DECLARE_OP(andOp,		"&&", 38, OperatorData::BOTH,		false	);	\
DECLARE_OP(equal_,		"=", 40, OperatorData::BOTH,		true	);	\
DECLARE_OP(notEqual,	"!=", 40, OperatorData::BOTH,		false	);	\
DECLARE_OP(greater_,	">", 50, OperatorData::BOTH,		true	);	\
DECLARE_OP(less_,		"<", 50, OperatorData::BOTH,		true	);	\
DECLARE_OP(greaterEq,	">=", 50, OperatorData::BOTH,		true	);	\
DECLARE_OP(lessEq,		"<=", 50, OperatorData::BOTH,		true	);	\
DECLARE_OP(plus_,		"+", 61, OperatorData::BOTH,		true	);	\
DECLARE_OP(minus_,		"-", 61, OperatorData::BOTH,		true	);	\
DECLARE_OP(multiply_,	"*", 71, OperatorData::BOTH,		true	);	\
DECLARE_OP(divide_,		"/", 71, OperatorData::BOTH,		true	);	\
DECLARE_OP(mod_,		"%", 70, OperatorData::BOTH,		true	);	\
DECLARE_OP(notOp_,		"!", 74, OperatorData::RIGHT,	true	);	\
DECLARE_OP(plusPlus_,	"++", 75, OperatorData::LEFT,		false	);	\
DECLARE_OP(minusMinus_,	"--", 75, OperatorData::LEFT,		false	);	\
DECLARE_OP(dot_,		".", 81, OperatorData::BOTH,		false	);	\
DECLARE_OP(rightArrow_,	"->", 83, OperatorData::BOTH,		false	);	\
DECLARE_OP(import_,		"==>", 90, OperatorData::RIGHT,	false	);	\
DECLARE_OP(open_paren_,	"(", 100, OperatorData::RIGHT,	false	);	\
DECLARE_OP(close_paren_,	")", 99, OperatorData::LEFT,		false	);	\
DECLARE_OP(open_sq_brac_,	"[", 100, OperatorData::BOTH,		false	);	\
DECLARE_OP(close_sq_brac_, "]", 99, OperatorData::LEFT,		false	);	\
DECLARE_OP(open_cr_brac_,	"{", 100, OperatorData::RIGHT,	false	);	\
DECLARE_OP(close_cr_brac_, "}", 99, OperatorData::LEFT,		false	);	\
  ALL_OPS;

  std::unordered_map<std::string, Operator> &GetOpsMap() { return ops_map_; }

  bool IsOpenBrac(Operator op);
  bool IsCloseBrac(Operator op);
private:
  AllOperators();

  void PutOpInMap(Operator op);

  std::unordered_map<std::string, Operator> ops_map_;
};

extern AllOperators *ops_;

#endif // TEREBINTH_ALL_OPERATORS_H_
