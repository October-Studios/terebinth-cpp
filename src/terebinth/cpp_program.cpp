#include "cpp_program.h"

#include <map>
#include <unordered_set>

#include "VERSION.h"
#include "error_handler.h"
#include "util/string_utils.h"

std::string GetValidCppId(std::string in) {
  std::string cpp;

  if (in.empty()) {
    in = "no_name";
  }

  int start = 0;

  do {
    int i = start;

    for (; i < int(in.size()) &&
           ((in[i] >= 'a' && in[i] <= 'z') || (in[i] >= 'A' && in[i] <= 'Z') ||
            (in[i] >= '0' && in[i] <= '9') || in[i] == '_');
         ++i) {
    }

    if (i != start) {
      if (in[start] >= '0' && in[start] <= '9') {
        cpp += "_";
      }
      cpp += in.substr(start, i - start);
    }

    start = i + 1;
  } while (start < int(in.size()));

  return cpp;
}

//
// Name Container
//

CppNameContainer::CppNameContainer() {}

std::shared_ptr<CppNameContainer> CppNameContainer::MakeRoot() {
  auto out = std::shared_ptr<CppNameContainer>(new CppNameContainer());
  out->parent_ = nullptr;
  return out;
}

std::shared_ptr<CppNameContainer> CppNameContainer::MakeChild() {
  auto out = std::shared_ptr<CppNameContainer>(new CppNameContainer());
  children_.push_back(out);
  out->parent_ = this;
  return out;
}

void CppNameContainer::AddTb(const std::string &tb,
                             const std::string &cpp_name_hint) {
  std::string valid_cpp_hint;

  if (cpp_name_hint == "<- the value of that pn string please")
    valid_cpp_hint = GetValidCppId(tb);
  else if (!cpp_name_hint.empty())
    valid_cpp_hint = GetValidCppId(cpp_name_hint);

  if (tb_to_cpp_map_.find(tb) != tb_to_cpp_map_.end()) {
    throw TerebinthError("Tried to add '" + tb +
                             "' as a pn name to a CppNameContainer but that pn "
                             "name already exists",
                         INTERNAL_ERROR);
  }

  std::string cpp;

  if (valid_cpp_hint.empty()) {
    cpp = str::GetUniqueString(
        "nm", [this](std::string in) -> bool { return !HasCpp(in); }, true);
  } else {
    cpp = str::GetUniqueString(
        valid_cpp_hint, [this](std::string in) -> bool { return !HasCpp(in); },
        false);
  }

  tb_to_cpp_map_[tb] = cpp;
  cpp_set_.insert(cpp);
}

void CppNameContainer::ReserveCpp(const std::string &cpp,
                                  bool ignore_collisions) {
  if (!ignore_collisions && HasCpp(cpp)) {
    throw TerebinthError("called CppNameContainer::ReserveCpp with id '" + cpp +
                             "', which already exists",
                         INTERNAL_ERROR);
  }

  cpp_set_.insert(cpp);
}

bool CppNameContainer::HasTbMe(const std::string &tb) {
  return tb_to_cpp_map_.find(tb) != tb_to_cpp_map_.end();
}

bool CppNameContainer::HasTb(const std::string &tb) {
  return HasTbMe(tb) || (parent_ && parent_->HasTb(tb));
}

bool CppNameContainer::HasCpp(const std::string &cpp) {
  return HasCppMe(cpp) || HasCppUp(cpp) || HasCppDown(cpp);
}

bool CppNameContainer::HasCppMe(const std::string &cpp) {
  return cpp_set_.find(cpp) != cpp_set_.end();
}

bool CppNameContainer::HasCppUp(const std::string &cpp) {
  return parent_ && (parent_->HasCppMe(cpp) || parent_->HasCppUp(cpp));
}

bool CppNameContainer::HasCppDown(const std::string &cpp) {
  for (auto i : children_) {
    if (i->HasCppMe(cpp) || i->HasCppDown(cpp)) {
      return true;
    }
  }

  return false;
}

std::string CppNameContainer::GetCpp(const std::string &tb) {
  auto result = tb_to_cpp_map_.find(tb);

  if (result == tb_to_cpp_map_.end()) {
    if (parent_)
      return parent_->GetCpp(tb);
    else
      throw TerebinthError("could not find C++ equivalent of '" + tb +
                               "' in CppNameContainer::GetCppForTb",
                           INTERNAL_ERROR);
  } else {
    return result->second;
  }
}

//
// Function Base
//

CppFuncBase::CppFuncBase(std::string prototype_in,
                         std::shared_ptr<CppNameContainer> my_names,
                         bool returns_val_in) {
  prototype = prototype_in;
  namespace_stack.push_back(my_names);
  fresh_line = true;
  returns_val = returns_val_in;
}

void CppFuncBase::Code(const std::string &in) {
  if (fresh_line) {
    source += str::IndentString(in, indent);
    fresh_line = (in.back() == '\n');
  } else if (str::SearchInString(in, "\n", 0) > 0) {
    source += "\n" + str::IndentString(in, indent);
    if (in.back() != '\n') source += "\n";
    fresh_line = true;
  } else {
    source += in;
    fresh_line = (in.back() == '\n');
  }
}

void CppFuncBase::Name(const std::string &in) {
  Code(namespace_stack.back()->GetCpp(in));
}

void CppFuncBase::Line(const std::string &in) {
  Code(in);
  Endln();
}

void CppFuncBase::Endln() {
  if (expr_level > 0) {
    throw TerebinthError(
        "non zero expression level when ending line in C++ "
        "program, code so far:\n" +
            str::IndentString(source, " "),
        INTERNAL_ERROR);
  } else if (fresh_line &&
             (source.size() < 2 || source[source.size() - 2] == ';' ||
              source[source.size() - 2] == '}' ||
              source[source.size() - 2] == '{')) {
    // NOOP
  } else {
    source += ";\n";
  }

  fresh_line = true;
}

void CppFuncBase::Comment(const std::string &in) {
  if (str::SearchInString(in, "\n", 0) >= 0) {
    source += str::IndentString("\n/*\n" + in + "\n*/\n", indent);
    fresh_line = true;
  } else if (expr_level > 0 || !fresh_line) {
    source += "/* " + in + " */";
    fresh_line = false;
  } else {
    source += str::IndentString("// " + in + "\n", indent);
    fresh_line = true;
  }
}

void CppFuncBase::PushExpr() {
  Code("(");
  expr_level++;
  fresh_line = false;
}

void CppFuncBase::PopExpr() {
  if (expr_level <= 0) {
    throw TerebinthError(
        "CppProgram::PopExpr called with zero expression level",
        INTERNAL_ERROR);
  }

  Code(")");
  expr_level--;
  fresh_line = false;
}

void CppFuncBase::PushBlock() {
  if (expr_level > 0) {
    throw TerebinthError(
        "CppProgram::PushBlock called when expressionLevel was not zero",
        INTERNAL_ERROR);
  }

  {
    Code("{\n");
    namespace_stack.push_back(namespace_stack.back()->MakeChild());
    block_level++;
    fresh_line = true;
  }
}

void CppFuncBase::PopBlock() {
  {
    if (block_level <= 0) {
      throw TerebinthError(
          "CppProgram::PopBlock called with zero indentationLevel",
          INTERNAL_ERROR);
    }

    block_level--;
    namespace_stack.pop_back();
    Code("}\n");
    fresh_line = true;
  }
}

std::string CppFuncBase::TbToCpp(const std::string &in) {
  return namespace_stack.back()->GetCpp(in);
}

//
// Program
//

CppProgram::CppProgram() {
  global_names = CppNameContainer::MakeRoot();
  Setup();
}

void CppProgram::Setup() {
  global_top_code += "// this C++ code is transpiled from Terebinth\n";
  global_top_code += "// Terebinth v" + std::to_string(VERSION_MAJOR) + "." +
                     std::to_string(VERSION_MINOR) + "." +
                     std::to_string(VERSION_PATCH) + " was used\n";
  global_includes_code += "#include <string.h>\n";
  global_includes_code += "#include <stdlib.h>\n";
  global_includes_code += "#include <stdio.h>\n";

  global_var_code += "int argc = 0;\n";
  global_var_code += "char** argv = 0;\n";

  std::vector<std::string> cpp_reserved_words{
      // from C
      "auto",
      "const",
      "double",
      "float",
      "int",
      "short",
      "struct",
      "unsigned",
      "break",
      "continue",
      "else",
      "for",
      "long",
      "signed",
      "switch",
      "void",
      "case",
      "default",
      "enum",
      "goto",
      "register",
      "sizeof",
      "typedef",
      "volatile",
      "char",
      "do",
      "extern",
      "if",
      "return",
      "static",
      "union",
      "while",

      // from old C++
      "asm",
      "dynamic_cast",
      "namespace",
      "reinterpret_cast",
      "try",
      "bool",
      "explicit",
      "new",
      "static_cast",
      "typeid",
      "catch",
      "false",
      "operator",
      "template",
      "typename",
      "class",
      "friend",
      "private",
      "this",
      "using",
      "const_cast",
      "inline",
      "public",
      "throw",
      "virtual",
      "delete",
      "mutable",
      "protected",
      "true",
      "wchar_t",

      // from C++11
      "and",
      "bitand",
      "compl",
      "not_eq",
      "or_eq",
      "xor_eq",
      "and_eq",
      "bitor",
      "not",
      "or",
      "xor",

      // something else
      "endl",
      "INT_MIN",
      "std",
      "INT_MAX",
      "MAX_RAND",
      "NULL",

      // my custom
      "main",
      "argc",
      "argv",
  };

  for (auto i : cpp_reserved_words) {
    global_names->ReserveCpp(i);
  }

  PushFunc(std::string("_main"), Void, Void, Void);
}

std::string CppProgram::GetTypeCode(Type in) {
  switch (in->GetType()) {
    case TypeBase::VOID:
      return "void";

    case TypeBase::DOUBLE:
      return "double";

    case TypeBase::INT:
      return "int";

    case TypeBase::BYTE:
      return "unsigned char";

    case TypeBase::BOOL:
      return "bool";

    case TypeBase::PTR:
      if (in->GetSubType()->IsWhatev())
        return "void *";
      else
        return GetTypeCode(in->GetSubType()) + " *";

    case TypeBase::TUPLE: {
      if (in->GetAllSubTypes()->size() == 1)
        return GetTypeCode((*in->GetAllSubTypes())[0].type);

      std::string compact = "{" + in->GetCompactString() + "}";

      if (!global_names->HasTb(compact)) {
        global_names->AddTb(compact, in->name_hint_);
        std::string code;
        code += "struct ";
        code += global_names->GetCpp(compact);
        code += "\n{\n";
        for (auto i : *in->GetAllSubTypes()) {
          code += str::IndentString(GetTypeCode(i.type) + " " + i.name + ";\n",
                                    indent);
        }

        code += "\n";
        code += str::IndentString(global_names->GetCpp(compact), indent);
        code += "() {}\n";

        code += "\n";
        auto con_names = global_names->MakeChild();
        code += str::IndentString(global_names->GetCpp(compact), indent);
        code += "(";
        bool first = true;
        for (auto i : *in->GetAllSubTypes()) {
          if (first) {
            first = false;
          } else {
            code += ", ";
          }
          con_names->AddTb("+" + i.name + " _in");
          code += GetTypeCode(i.type) + " " +
                  con_names->GetCpp("+" + i.name + "_in");
        }
        code += ")\n";
        code += str::IndentString("{\n", indent);
        for (auto i : *in->GetAllSubTypes()) {
          code += str::IndentString(
              i.name + " = " + con_names->GetCpp("+" + i.name + "_in") + " ;\n",
              indent);
        }
        code += str::IndentString("}\n", indent);

        code += "};\n";

        if (!global_types_code.empty()) global_types_code += "\n";
        global_types_code += code;
      }

      return global_names->GetCpp(compact);
    }

    default:
      throw TerebinthError("CppProgram::getTypeCode called with invalid type " +
                               (TypeBase::GetString(in->GetType())),
                           INTERNAL_ERROR);
  }
}

void CppProgram::DeclareVar(const std::string &name_in, Type type_in,
                            std::string initial_value) {
  if (IsMain()) {
    DeclareGlobal(name_in, type_in, initial_value);
    return;
  }

  if (active_func->namespace_stack.back()->HasTb(name_in)) {
    return;
  }

  active_func->namespace_stack.back()->AddTb(name_in);
  active_func->namespace_stack[0]->ReserveCpp(TbToCpp(name_in), true);

  active_func->var_declare_source +=
      GetTypeCode(type_in) + " " + TbToCpp(name_in);
  if (!initial_value.empty()) {
    active_func->var_declare_source += " = " + initial_value;
  }
  active_func->var_declare_source += ";\n";
}

void CppProgram::DeclareGlobal(const std::string &name_in, Type type_in,
                               std::string initial_value) {
  if (global_names->HasTb(name_in)) {
    return;
  }

  std::string code;
  code += GetTypeCode(type_in);
  code += " ";
  global_names->AddTb(name_in);
  code += global_names->GetCpp(name_in);
  if (!initial_value.empty()) {
    code += " = ";
    code += initial_value;
  }
  code += ";\n";

  global_var_code += code;
}

void CppProgram::AddHeadCode(const std::string &code) {
  global_includes_code += code + "\n";
}

bool CppProgram::HasFunc(const std::string &name) {
  return funcs.find(name) != funcs.end();
}

void CppProgram::AddFunc(const std::string &name,
                         std::vector<std::pair<std::string, std::string>> args,
                         std::string return_type, std::string contents) {
  if (HasFunc(name)) {
    TerebinthError("called CppProgram::AddFunc with function name '" + name +
                       "', which already exists",
                   INTERNAL_ERROR);
  }

  global_names->AddTb(name, name);
  std::string cpp_name = global_names->GetCpp(name);

  std::string prototype;
  prototype += return_type;
  prototype += " " + cpp_name + "(";

  for (auto i = 0; i < int(args.size()); ++i) {
    if (i) {
      prototype += ", ";
    }
    prototype += args[i].first;
    prototype += " ";
    prototype += args[i].second;
  }

  prototype += ")";

  auto func =
      CppFunc(new CppFuncBase(prototype, global_names->MakeChild(),
                              (return_type != "" && return_type != "void")));
  funcs[name] = func;
  func->Code(contents);
}

void CppProgram::PushFunc(const std::string &name,
                          const std::string &cpp_name_hint, Type left_in,
                          Type right_in, Type return_type) {
  if (HasFunc(name)) {
    throw TerebinthError("called CppProgram::PushFunc with function name '" +
                             name + "', which already exists",
                         INTERNAL_ERROR);
  }

  global_names->AddTb(name, cpp_name_hint);

  std::string cpp_name = global_names->GetCpp(name);
  auto func_names = global_names->MakeChild();

  std::string prototype;
  prototype += GetTypeCode(return_type);

  prototype += " " + cpp_name + "(";

  bool keep_tuples_together = true;

  if (keep_tuples_together) {
    if (left_in->IsCreatable()) {
      prototype += GetTypeCode(left_in) + " me";
      func_names->AddTb("me");
    }

    if (right_in->IsCreatable()) {
      if (left_in->IsCreatable()) {
        prototype += ", ";
      }

      prototype += GetTypeCode(right_in) + " in";
      func_names->AddTb("in");
    }
  } else {
    std::vector<std::pair<std::string, std::string>> args;
    if (left_in->GetType() == TypeBase::TUPLE) {
      for (auto i : *left_in->GetAllSubTypes()) {
        args.push_back({GetTypeCode(i.type), i.name});
      }
    } else if (!left_in->IsCreatable()) {
      // NOOP
    } else {
      args.push_back({GetTypeCode(left_in), "me"});
    }

    if (right_in->GetType() == TypeBase::TUPLE) {
      for (auto i : *right_in->GetAllSubTypes()) {
        args.push_back({GetTypeCode(i.type), i.name});
      }
    } else if (!right_in->IsCreatable()) {
      // NOOP
    } else {
      args.push_back({GetTypeCode(right_in), "in"});
    }

    for (auto i = 0; i < int(args.size()); ++i) {
      if (i) {
        prototype += ", ";
      }

      prototype += args[i].first;
      prototype += " ";
      func_names->AddTb(args[i].second, args[i].second);
      prototype += func_names->GetCpp(args[i].second);
    }
  }

  prototype += ")";

  active_func = CppFunc(
      new CppFuncBase(prototype, func_names, return_type->IsCreatable()));
  funcs[name] = active_func;
  func_stack.push_back(name);
}

void CppProgram::PopFunc() {
  if (active_func->GetExprLevel() > 0 || !active_func->GetIfFreshLine() ||
      active_func->GetBlockLevel() > 0) {
    throw TerebinthError(
        "called CppProgram::PopFunc when function was not ready",
        INTERNAL_ERROR);
  }

  func_stack.pop_back();

  if (func_stack.empty()) {
    throw TerebinthError("called CppProgram::PopFunc too many times",
                         INTERNAL_ERROR);
  }

  active_func = funcs[func_stack.back()];
}

std::string CppProgram::GetCppCode() {
  std::string out;

  if (!global_top_code.empty()) {
    out += global_top_code + "\n";
  }

  if (!global_includes_code.empty()) {
    out += global_includes_code + "\n";
  }

  if (!global_types_code.empty()) {
    out += global_types_code + "\n";
  }

  if (funcs.size() > 1) {
    for (auto i : funcs) {
      if (i.first != "main") {
        out += i.second->GetPrototype() + ";\n";
      }
    }

    out += "\n";
  }

  if (!global_var_code.empty()) {
    out += global_var_code + "\n";
  }

  for (auto i : funcs) {
    out += i.second->GetPrototype();

    std::string func_src = i.second->GetSource();

    if (func_src.size() < 2) {
      out += "\n{\n\t// empty function\n}\n\n";
    } else {
      if (i.second->GetIfReturnsVal() && func_src[0] != '{' &&
          str::SearchInString(func_src, ";", 0) == int(func_src.size()) - 2) {
        func_src = "return " + func_src;
      }
      out += "\n{\n";
      if (!i.second->var_declare_source.empty()) {
        out += str::IndentString(i.second->var_declare_source + "\n", indent);
      }

      out += str::IndentString(func_src, indent);
      if (!out.empty() && out.back() != '\n') {
        out += ";\n";
      }

      out += "}\n\n";
    }
  }

  out +=
      "int main(int argcIn, char** argvIn) {\n\
  argc = argcIn;\n\
	argv = argvIn;\n\
	if (argc >= 2 && strcmp(argv[1], \"--running-from-terebinth\") == 0) {\n\
	  argc -= 2;\n\
	  if (argc == 0)\n\
		  argv = 0;\n\
	  else\n\
		  argv += 2;\n\
	}\n\
	_main();\n\
	return 0;\n\
}\n";

  return out;
}
