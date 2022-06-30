#ifndef TEREBINTH_CPP_PROGRAM_H_
#define TEREBINTH_CPP_PROGRAM_H_

#include "type.h"

#include <map>

#include <memory>
#include <unordered_set>

class CppNameContainer {
public:
  static std::shared_ptr<CppNameContainer> MakeRoot();
  std::shared_ptr<CppNameContainer> MakeChild();
  void AddTb(const std::string &t, const std::string &cpp_name_hint =
                                      "<- the value of that t string please");
  void ReserveCpp(const std::string &cpp, bool ignore_collision = false);
  bool HasTb(const std::string &t);
  std::string GetCpp(const std::string &t);
  CppNameContainer *GetParent() { return parent_; }

private:
  bool HasTbMe(const std::string &t);
  CppNameContainer();
  bool HasCpp(const std::string &cpp);
  bool HasCppMe(const std::string &cpp);
  bool HasCppUp(const std::string &cpp);
  bool HasCppDown(const std::string &cpp);

  std::unordered_set<std::string> cpp_set_;
  std::map<std::string, std::string> tb_to_cpp_map_;
  CppNameContainer *parent_ = nullptr;
  std::vector<std::shared_ptr<CppNameContainer>> children_;
};

class CppFuncBase {
public:
  CppFuncBase(std::string prototype_in,
              std::shared_ptr<CppNameContainer> my_names, bool returns_val_in);

  void Code(const std::string &in);
  void Name(const std::string &in);
  void Line(const std::string &in);
  void Endln();
  void Comment(const std::string &in);
  void PushExpr();
  void PopExpr();
  void PushBlock();
  void PopBlock();
  std::string TbToCpp(const std::string &in);
  int GetExprLevel() { return expr_level; }
  bool GetIfFreshLine() { return fresh_line; }
  int GetBlockLevel() { return block_level; }
  bool GetIfReturnsVal() { return returns_val; }

  std::string GetSource() { return source; }
  std::string GetPrototype() { return prototype; }

private:
  std::string indent = "\t";
  bool fresh_line = true;
  int block_level = 0;
  int expr_level = 0;

  std::string var_declare_source;
  std::string source;
  std::string prototype;
  bool returns_val = false;
  bool fake_start_block = false;

  std::vector<std::shared_ptr<CppNameContainer>> namespace_stack;

  friend CppProgram;
};

typedef std::shared_ptr<CppFuncBase> CppFunc;

class CppProgram {
public:
  CppProgram();

  void Code(const std::string &in) { active_func->Code(in); }
  void Name(const std::string &in) { active_func->Name(in); }
  void Line(const std::string &in) { active_func->Line(in); }
  void Endln() { active_func->Endln(); }
  void Comment(const std::string &in) { active_func->Comment(in); }
  void PushExpr() { active_func->PushExpr(); }
  void PopExpr() { active_func->PopExpr(); }
  void PushBlock() { active_func->PushBlock(); }
  void PopBlock() { active_func->PopBlock(); }
  std::string TbToCpp(const std::string &in) { return active_func->TbToCpp(in); }
  int GetExprLevel() { return active_func->GetExprLevel(); }
  int GetBlockLevel() { return active_func->GetBlockLevel(); }
  int GetIfReturnsVal() { return active_func->GetIfReturnsVal(); }

  void Setup();
  std::string GetTypeCode(Type in);
  void DeclareVar(const std::string &name_in, Type type_in,
                  std::string initial_value = "");
  void DeclareGlobal(const std::string &name_in, Type type_in,
                     std::string initial_value = "");
  void AddHeadCode(const std::string &code);
  bool HasFunc(const std::string &name);
  void AddFunc(const std::string &name,
               std::vector<std::pair<std::string, std::string>> args,
               std::string return_type, std::string contents);
  void PushFunc(const std::string &name, const std::string &cpp_name_hint,
                Type left_in, Type right_in, Type return_type);
  void PushFunc(const std::string &name, Type left_in, Type right_in,
                Type return_type) {
    PushFunc(name, name, left_in, right_in, return_type);
  }
  void PopFunc();
  bool IsMain() { return func_stack.size() == 1; }

  std::string GetCppCode();

  std::shared_ptr<CppNameContainer> GetGlobalNames() { return global_names; };

private:
  std::string indent = "\t";
  std::string global_top_code;
  std::string global_includes_code;
  std::string global_var_code;
  std::string global_types_code;
  CppFunc active_func;
  std::vector<std::string> func_stack;
  std::map<std::string, CppFunc> funcs;
  std::shared_ptr<CppNameContainer> global_names;
};

void AddToProgTbStr(CppProgram *prog);

#endif // TEREBINTH_CPP_PROGRAM_H_
