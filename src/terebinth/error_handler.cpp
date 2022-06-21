#include "error_handler.h"
#include "source_file.h"

#include <iostream>

ErrorHandler error_;

std::string ErrorHandler::PriorityToStr(ErrorPriority in) {
  switch (in) {
  case SOURCE_ERROR:
    return "error";
    break;
  case SOURCE_WARNING:
    return "warning";
    break;
  case JSYK:
    return "jsyk";
    break;
  case INTERNAL_ERROR:
    return "INTERNAL ERROR";
    break;
  case RUNTIME_ERROR:
    return "runtime error";
    break;
  default:
    return "UNKNOWN PRIORITY LEVEL";
    break;
  }
}

void ErrorHandler::Log(std::string msg, ErrorPriority priority, Token token) {
    if (priority == SOURCE_ERROR || priority == INTERNAL_ERROR || priority == RUNTIME_ERROR) {
        error_has_been_logged_ = true;
    }

    std::cout << PriorityToStr(priority);
}