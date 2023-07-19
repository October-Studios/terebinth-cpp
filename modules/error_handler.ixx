export module error_handler;

import <string>;

import token;

export {
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
  std::string msg_;
  ErrorPriority priority_;
  Token token_;
};

}  // export
