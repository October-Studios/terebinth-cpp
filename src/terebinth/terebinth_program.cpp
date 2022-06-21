#include "terebinth_program.h"
#include "all_operators.h"
#include "cpp_program.h"
#include "error_handler.h"
#include "operator.h"
#include <memory>

void PopulateTerebinthStdLib();
void LexString(std::shared_ptr<SourceFile> file, std::vector<Token> &tokens);
Action ParseFunction(const std::vector<Token> &tokens, int left, int right,
                     Type left_in_type, Type right_in_type);

extern Namespace global_namespace;

TerebinthProgram::TerebinthProgram() {}

void TerebinthProgram::CleanUp() {}

void TerebinthProgram::ResolveProgram(std::string in_filename,
                                      bool print_output) {
  AllOperators::Init();
  PopulateTerebinthStdLib();

  if (!error_.GetIfErrorLogged()) {
    try {
      file_ = std::shared_ptr<SourceFile>(
          new SourceFile(in_filename, print_output));

      if (print_output) {
        std::cout << std::endl
                  << std::endl
                  << file_->GetBoxedString() << std::endl;
      }
    } catch (TerebinthError err) {
      err.Log();
    }
  }

  if (!error_.GetIfErrorLogged()) {
    try {
      LexString(file_, tokens_);
    } catch (TerebinthError err) {
      err.Log();
      ast_root_ = AstVoid::Make();
    }
  }

  if (!error_.GetIfErrorLogged()) {
    try {
      ast_root_ = AstNodeFromTokens(tokens_, 0, tokens_.size() - 1);
    } catch (TerebinthError err) {
      err.Log();
      ast_root_ = AstVoid::Make();
    }

    if (print_output) {
      std::cout << " ╭──────────────────────╮" << std::endl;
      std::cout << " │ abstract syntax tree │" << std::endl;
      std::cout << " ╰──────────────────────╯" << std::endl;
      std::cout << ast_root_->GetString() << std::endl;
    }
  }

  if (!error_.GetIfErrorLogged()) {
    try {
      ast_root_->SetInput(global_namespace, true, Void, Void);
    } catch (TerebinthError err) {
      err.Log();
      ast_root_ = AstVoid::Make();
    }

    try {
      action_root_ = ast_root_->GetAction();

      if (print_output) {
        std::cout << " ╭─────────────╮" << std::endl;
        std::cout << " │ action tree │" << std::endl;
        std::cout << " ╰─────────────╯" << std::endl;
        std::cout << action_root_->GetDescription() << std::endl;
      }
    } catch (TerebinthError err) {
      err.Log();
    }
  }
}

std::string TerebinthProgram::GetCpp() {
  try {
    CppProgram out_program;
    action_root_->AddToProg(void_action_, void_action_, &out_program);
    return out_program.GetCppCode();
  } catch (TerebinthError err) {
    err.Log();
    return "/* no program due to transpilation error */";
  }
}

void TerebinthProgram::Execute() {
  try {
    stack_ptr = global_frame_ptr =
        malloc(global_namespace->GetStackFrame()->GetSize());
    free(action_root_->Execute(nullptr, nullptr));
    free(global_frame_ptr);
    stack_ptr = global_frame_ptr = nullptr;
  } catch (TerebinthError err) {
    err.Log();
    std::cout << std::endl
              << ">>>>>>   program aborted due to error    <<<<<<" << std::endl;
  }
}