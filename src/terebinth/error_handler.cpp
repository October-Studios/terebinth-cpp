#include "error_handler.h"
#include "source_file.h"
#include "string_utils.h"

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
  if (priority == SOURCE_ERROR || priority == INTERNAL_ERROR ||
      priority == RUNTIME_ERROR) {
    error_has_been_logged_ = true;
  }

  std::cout << PriorityToStr(priority);

  if (token) {
    std::cout << " in '" << token->GetFile()->GetFilename() << "' on line "
              << token->GetLine() << ":\n";
    std::cout << str::IndentString(msg, "    ") << std::endl;

    std::string line = token->GetFile()->GetLine(token->GetLine());

    int wspace = 0;
    for (;
         wspace < int(line.size()) &&
         (line[wspace] == ' ' || line[wspace] == '\t' || line[wspace] == '\n');
         wspace++) {
    }

    std::string arrows = "";
    for (auto i = 0; i < token->GetCharPos() - 1 - wspace; ++i) {
      arrows += " ";
    }
    for (auto i = 0; i < int(token->GetText().size()); ++i) {
      arrows += "^";
    }

    std::cout << str::IndentString("" + line.substr(wspace, std::string::npos) +
                                       "\n" + arrows,
                                   "      ")
              << std::endl;
  } else {
    std::cout << ": " << msg << std::endl;
  }
}

void ErrorHandler::Msg(std::string in) {
  std::cout << "message: " << in << "\n";
}

TerebinthError::TerebinthError(std::string msg_in, ErrorPriority priority_in,
                               Token token_in) {
  msg_ = msg_in;
  priority_ = priority_in;
  token_ = token_in;
}

void TerebinthError::Log() { error_.Log(msg_, priority_, token_); }
