#include "VERSION.h"

#include <iostream>
#include <list>
#include <math.h>
#include <string>
#include <vector>

#include "ast_node.h"
#include "namespace.h"
#include "source_file.h"
#include "stack_frame.h"
#include "token.h"

extern std::vector<std::string> cmd_line_args;

class Element;

class TerebinthProgram {
public:
  TerebinthProgram();
  ~TerebinthProgram() { CleanUp(); }

  std::string GetCpp();

  void ResolveProgram(std::string in_filename, bool print_extra_output);

  Namespace GetGlobalActionTable();

  void Execute();

private:
  void CleanUp();

private:
  std::shared_ptr<SourceFile> file_ = nullptr;
  std::vector<Token> tokens_;
  AstNode ast_root_ = nullptr;
  Action action_root_ = CreateNewVoidAction();
  std::vector<char> whitespace_chars_, letter_chars_, digit_chars_;
  char single_line_comment_;
};
