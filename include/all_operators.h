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

#define ALL_OPS DECLARE_OP(loop, "@", 5, OperatorData::BOTH, false);

  ALL_OPS;

  std::unordered_map<std::string, Operator> &GetOpsMap() { return ops_map_; }

private:
  AllOperators();

  void PutOpInMap(Operator op);

  std::unordered_map<std::string, Operator> ops_map_;
};

extern AllOperators *ops_;

#endif // TEREBINTH_ALL_OPERATORS_H_