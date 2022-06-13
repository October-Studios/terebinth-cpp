#ifndef TEREBINTH_ERROR_HANDLER_H_
#define TEREBINTH_ERROR_HANDLER_H_

#include "token.h"
#include <string>

#define FUNC std::string(__FUNCTION__)

enum ErrorPriority {
  SOURCE_ERROR,
  SOURCE_WARNING,
  JSYK,
  INTERNAL_ERROR,
  RUNTIME_ERROR
};

class ErrorHandler {
public:
  static std::string PriorityToStr(ErrorPriority in);

  void Log(std::string msg, ErrorPriority priority, Token token = nullptr);

  void Msg(std::string in);

  bool GetIfErrorLogged() { return error_has_been_logged_; }

private:
  bool error_has_been_logged_;
};

extern ErrorHandler error_;

class TerebinthError {
public:
  TerebinthError(std::string msg_in, ErrorPriority priority_in,
                 Token token_in = nullptr);

  void Log();

private:
  std::string msg;
  ErrorPriority priority;
  Token token;
};

#endif // TEREBINTH_ERROR_HANDLER_H_
