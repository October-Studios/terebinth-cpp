#ifndef TEREBINTH_OPERATOR_H_
#define TEREBINTH_OPERATOR_H_

class TokenData;

#include <memory>
#include <string>

class AllOperators;

class OperatorData {
public:
  enum InputTaken { LEFT, RIGHT, BOTH };

  std::string GetText() { return text_; }
  int GetPrecedence() { return precedence_; }
  bool IsOverloadable() { return overloadable_; }
  bool TakesLeftInput() { return input_ == BOTH || input_ == LEFT; }
  bool TakesRightInput() { return input_ == BOTH || input_ == RIGHT; }

private:
  friend AllOperators;

  OperatorData(std::string text_in, int precedence_in, InputTaken input_in,
               bool overloadable_in) {
    text_ = text_in;
    precedence_ = precedence_in;
    input_ = input_in;
    overloadable_ = overloadable_in;
  }

  std::string text_;

  int precedence_;

  bool overloadable_;

  InputTaken input_;
};

typedef std::shared_ptr<OperatorData> Operator;

#endif // TEREBINTH_OPERATOR_H_