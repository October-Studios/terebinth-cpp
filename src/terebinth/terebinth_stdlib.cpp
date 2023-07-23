module;
import <ios>;

import action;
import all_operators;
import ast_node;
import cpp_program;
import error_handler;
import Namespace;
import stack_frame;
import terebinth_program;
import type;
import util.string_utils;

#define CONCAT(a, b) a##_##b
#define GET_TYPES_Tuple(t0, t1) t0, t1

#define assert if (
#define otherwise ) {} \
  else

#define GetTerType(type_in) CONCAT(TER, type_in)
#define GetCppType(type_in) CONCAT(CPP, type_in)

#define CPP_Double double
#define CPP_Byte unsigned char
#define CPP_Int int
#define CPP_Bool bool
#define CPP_Void char

#define TER_String String
#define TER_Double Double
#define TER_Int Int
#define TER_Byte Byte
#define TER_Bool Bool
#define TER_Void Void

#define TER_Tuple(t1, t2) \
  MakeTuple(std::vector<NamedType>({{"a", t1}, {"b", t2}}), true)

#define LAMBDA_HEADER [](void* left_in, void* right_in) -> void*
#define ADD_CPP_HEADER [](Action left, Action right, CppProgram * prog) -> void

#define retrn out =
#define GET_PTR_VAL(type_in, var_in_name) \
  = *((GetCppType(type_in)*)(var_in_name))

#define DO_INSTANTIATE(type_in, var_out_name, val_in) \
  GetCppType(type_in) var_out_name val_in;
#define DONT_INSTANTIATE(type_in, var_out_name, val_in) ;
#define INSTANTIATE_CPP_TUPLE(t0, t1, var_out_name, val_in) \
  DO_INSTANTIATE(t0, CONCAT(var_out_name, 0), val_in)       \
  DO_INSTANTIATE(t1, CONCAT(var_out_name, 1),               \
                 ((char*)val_in) + sizeof(GetCppType(t0)))

#define DO_RETURN_VAL(type_in, var_name)                      \
  void* out_ptr = malloc(GetTerType(type_in)->GetSize());     \
  memcpy(out_ptr, &var_name, GetTerType(type_in)->GetSize()); \
  return out_ptr;
#define DONT_RETURN_VAL(type_in, var_name) return nullptr;

#define INSTANTIATE_String DO_INSTANTIATE
#define INSTANTIATE_Double DO_INSTANTIATE
#define INSTANTIATE_Int DO_INSTANTIATE
#define INSTANTIATE_Byte DO_INSTANTIATE
#define INSTANTIATE_Bool DO_INSTANTIATE
#define INSTANTIATE_Void DONT_INSTANTIATE
#define INSTANTIATE_Tuple__(type_in, var_out_name, val_in) \
  INSTANTIATE_CPP_TUPLE(type_in, var_out_name, val_in)
#define INSTANTIATE_Tuple_(type_in, var_out_name, val_in) \
  INSTANTIATE_Tuple__(GET_TYPES##_##type_in, var_out_name, val_in)
#define INSTANTIATE_Tuple(t1, t2) INSTANTIATE_Tuple_

#define RETURN_Double DO_RETURN_VAL
#define RETURN_Int DO_RETURN_VAL
#define RETURN_Byte DO_RETURN_VAL
#define RETURN_Bool DO_RETURN_VAL
#define RETURN_Void DONT_RETURN_VAL

#define func(name_text, left_type, right_type, return_type, lambda_body, cpp) \
  AddAction(                                                                  \
      name_text, GetTerType(left_type), GetTerType(right_type),               \
      GetTerType(return_type),                                                \
      LAMBDA_HEADER {                                                         \
        INSTANTIATE##_##left_type(left_type, left,                            \
                                  GET_PTR_VAL(left_type, left_in))            \
            INSTANTIATE##_##right_type(right_type, right,                     \
                                       GET_PTR_VAL(right_type, right_in))     \
                INSTANTIATE##_##return_type(return_type, out, ;) lambda_body; \
        CONCAT(RETURN, return_type)(return_type, out)                         \
      },                                                                      \
      cpp)

Action void_action_;

Namespace global_namespace_;
Namespace table;

auto GetText(Operator op) -> std::string { return op->GetText(); }
auto GetText(std::string in) -> std::string { return in; }

void AddConst(void* data, Type type, std::string name) {
  global_namespace_->AddNode(AstActionWrapper::Make(ConstGetActionT(
                                 data, type, name, global_namespace_)),
                             name);
}

template <typename T>
void AddAction(
    T id, Type left_type, Type right_type, Type return_type,
    std::function<void*(void*, void*)> lambda,
    std::function<void(Action in_left, Action in_right, CppProgram* prog)>
        cpp_writer) {
  global_namespace_->AddNode(
      AstActionWrapper::Make(LambdaActionT(left_type, right_type, return_type,
                                           lambda, cpp_writer, GetText(id))),
      GetText(id));
}

void AddType(Type type, std::string id) {
  auto node = AstTypeType::Make(type);
  node->SetInput(global_namespace_, false, Void, Void);
  global_namespace_->AddNode(std::move(node), id);
}

auto StringToLambda(std::string cpp_code)
    -> std::function<void(Action in_left, Action in_right, CppProgram* prog)> {
  if (cpp_code.empty()) {
    return nullptr;
  }

  return [=](Action in_left, Action in_right, CppProgram* prog) {
    int start = 0;
    int i;

    do {
      i = str::SearchInString(cpp_code, "$", start);

      if (i < 0) {
        prog->Code(cpp_code.substr(start, cpp_code.size() - start));
      } else {
        prog->Code(cpp_code.substr(start, i - start));

        if (str::SubMatches(cpp_code, i, "$.")) {
          prog->PushExpr();
          in_left->AddToProg(prog);
          start = i + std::string("$.").size();
          prog->PopExpr();
        } else if (str::SubMatches(cpp_code, i, "$:")) {
          prog->PushExpr();
          in_right->AddToProg(prog);
          start = i + std::string("$:").size();
          prog->PopExpr();
        } else if (str::SubMatches(cpp_code, i, "$$")) {
          prog->Code("$");
          start = i + std::string("$$").size();
        } else {
          throw TerebinthError("invalid '$' escape in C++ code: " + cpp_code,
                               INTERNAL_ERROR);
        }
      }
    } while (i >= 0);
  };
}

void AddAction(std::string text, Type left_type, Type right_type,
               Type return_type, std::function<void*(void*, void*)> lambda,
               std::string cpp) {
  AddAction(text, left_type, right_type, return_type, lambda,
            StringToLambda(cpp));
}

void AddAction(Operator op, Type left_type, Type right_type, Type return_type,
               std::function<void*(void*, void*)> lambda, std::string cpp) {
  AddAction(op, left_type, right_type, return_type, lambda,
            StringToLambda(cpp));
}

Type IntArray = nullptr;
Type Array = nullptr;

template <typename T>
inline auto GetValFromTuple(void* data, Type type, std::string name) -> T {
  while (type->GetType() == TypeBase::PTR) {
    type = type->GetSubType();
    data = *(void**)data;
  }

  OffsetAndType a = type->GetSubType(name);

  if (!a.type) {
    throw TerebinthError("tried to get invalid property '" + name +
                             "' from type " + type->GetString(),
                         INTERNAL_ERROR);
  }

  return *((T*)((char*)data + a.offset));
}

template <typename T>
inline void SetValInTuple(void* data, Type type, std::string name, T val) {
  while (type->GetType() == TypeBase::PTR) {
    type = type->GetSubType();
    data = *(void**)data;
  }

  OffsetAndType a = type->GetSubType(name);

  if (!a.type) {
    throw TerebinthError("tried to set invalid property '" + name +
                             "' from type " + type->GetString(),
                         INTERNAL_ERROR);
  }

  *((T*)((char*)data + a.offset)) = val;
}

inline auto TerStr2CppStr(void* obj) -> std::string {
  int len = GetValFromTuple<int>(obj, String, "_size");
  char* data = static_cast<char*>(malloc((len + 1) * sizeof(char)));
  memcpy(data, GetValFromTuple<char*>(obj, String, "_data"),
         len * sizeof(char));
  data[len] = 0;
  std::string out(data);
  return out;
}

inline auto CppStr2TerStr(std::string cpp) -> void* {
  void* obj = malloc(String->GetSize());
  char* str_data = static_cast<char*>(malloc(cpp.size() * sizeof(char)));
  memcpy(str_data, cpp.c_str(), cpp.size() * sizeof(char));

  *((int*)((char*)obj + String->GetSubType("_size").offset)) = cpp.size();
  *((char**)((char*)obj + String->GetSubType("_data").offset)) = str_data;

  return obj;
}

void AddToProgTbStr(CppProgram* prog) {
  if (!prog->HasFunc("$TbStr")) {
    std::string str_type = prog->GetTypeCode(String);

    prog->AddFunc("$TbStr", {{"const char *", "in"}}, str_type,
                  "int size = 0;\n"
                  "while (in[size]) size++;\n"
                  "return " +
                      str_type + "(size, unsigned char*)in);\n");
  }
}

void AddToProgCStr(CppProgram* prog) {
  if (!prog->HasFunc("$CStr")) {
    if (prog->HasFunc("$CStr")) {
      return;
    }

    std::string str_type = prog->GetTypeCode(String);

    prog->AddFunc("$CStr", {{str_type, "in"}}, "char *",
                  "char* out = (char*)malloc(in._size + 1);\n"
                  "memcpy(out, in._data, in._size);\n"
                  "out[in._size] = 0;\n"
                  "return out;\n");
  }
}

void AddToProgSubStr(CppProgram* prog) {}

void AddToProgIntToStr(CppProgram* prog) {}

void AddToProgStrToInt(CppProgram* prog) {}

void AddToProgStrToDouble(CppProgram* prog) {}

void AddToProgConcatStr(CppProgram* prog) {}

void AddToProgDoubleToStr(CppProgram* prog) {}

void AddToProgAsciiToStr(CppProgram* prog) {}

void AddToProgGetInputLine(CppProgram* prog) {}

void AddToProgEqStr(CppProgram* prog) {}

void AddToProgRunCmd(CppProgram* prog) {}

void AddToProgMakeIntArray(CppProgram* prog) {}

void AddToProgStrWithEscapedNames(CppProgram* prog, std::string str) {}

void BasicSetup() {
  table = global_namespace_ = NamespaceData::MakeRootNamespace();

  void_action_ = CreateNewVoidAction();
}

void PopulateBasicTypes() {
  String = MakeTuple(std::vector<NamedType>{NamedType{"_size", Int},
                                            NamedType{"_data", Byte->GetPtr()}},
                     false);

  AddType(Void, "Void");
  AddType(Bool, "Bool");
  AddType(Byte, "Byte");
  AddType(Int, "Int");
  AddType(Double, "Double");
  AddType(String, "String");
}

void PopulateConstants() {
  table->AddNode(AstActionWrapper::Make(void_action_), "void");

  bool true_val = true;
  AddConst(&true_val, Bool, "tru");

  bool false_val = false;
  AddConst(&false_val, Bool, "fls");

  // version constant
  {
    Type version_tuple_type = MakeTuple(
        std::vector<NamedType>{
            NamedType{"major", Int},
            NamedType{"minor", Int},
            NamedType{"patch", Int},
        },
        false);

    void* version_tuple_data = malloc(version_tuple_type->GetSize());

    SetValInTuple(version_tuple_data, version_tuple_type, "major",
                  Version::Major());
    SetValInTuple(version_tuple_data, version_tuple_type, "minor",
                  Version::Minor());
    SetValInTuple(version_tuple_data, version_tuple_type, "patch",
                  Version::Patch());

    AddConst(version_tuple_data, version_tuple_type, "VERSION");
  }

  // OS
  bool is_linux = false;
  bool is_windows = false;
  bool is_unix = false;
  bool is_mac = false;

#ifdef __linux__
  is_linux = true;
  is_unix = true;

#else

#ifdef _WIN32  // works for both 32 and 64 bit systems
  is_windows = true;

#else
#ifdef __APPLE__
  is_mac = true;
  is_unix = true;
#endif  // __APPLE__
#endif  // _WIN32

#endif  // __linux__

  AddConst(&is_linux, Bool, "OS_IS_LINUX");
  AddConst(&is_windows, Bool, "OS_IS_WINDOWS");
  AddConst(&is_mac, Bool, "OS_IS_MAC");
  AddConst(&is_unix, Bool, "OS_IS_UNIX");

  func("IS_TRANSPILED", Void, Void, Bool, retrn false;, "true");

  AddAction(
      "arg", Void, Int, String,
      LAMBDA_HEADER {
        int right = *(int*)right_in;
        if (right < (int)cmd_line_args.size()) {
          return CppStr2TerStr(cmd_line_args[right]);
        } else {
          return CppStr2TerStr("");
        }
      },
      ADD_CPP_HEADER {
        AddToProgTbStr(prog);

        prog->PushExpr();
        prog->PushExpr();
        prog->PushExpr();
        right->AddToProg(prog);
        prog->PopExpr();
        prog->Code(" < argc");
        prog->PopExpr();
        prog->Code("?");
        prog->PushExpr();
        prog->Name("$pnStr");
        prog->PushExpr();
        prog->Code("argv[");
        prog->PushExpr();
        right->AddToProg(prog);
        prog->PopExpr();
        prog->Code("]");
        prog->PopExpr();
        prog->PopExpr();
        prog->Code(":");
        prog->PushExpr();
        prog->Name("$pnStr");
        prog->PushExpr();
        prog->Code("\"\"");
        prog->PopExpr();
        prog->PopExpr();
        prog->PopExpr();
      });

  func("arg_len", Void, Void, Int, retrn cmd_line_args.size();, "argc");
}

void PopulateOperators() {
  /// +

  func(ops_->plus_, Int, Int, Int, retrn left + right;, "$. + $:");

  func(ops_->plus_, Double, Double, Double, retrn left + right;, "$. + $:");

  /// -

  func(ops_->minus_, Int, Int, Int, retrn left - right;, "$. - $:");

  func(ops_->minus_, Double, Double, Double, retrn left - right;, "$. - $:");

  func(ops_->minus_, Void, Int, Int, retrn - right;, "- $:");

  func(ops_->minus_, Void, Double, Double, retrn - right;, "- $:");

  /// *

  func(ops_->multiply_, Int, Int, Int, retrn left * right;, "$. * $:");

  func(ops_->multiply_, Double, Double, Double, retrn left * right;, "$. * $:");

  /// /

  func(ops_->divide_, Int, Int, Int, retrn left / right;, "$. / $:");

  func(ops_->divide_, Double, Double, Double, retrn left / right;, "$. / $:");

  /// %

  func(ops_->mod_, Int, Int, Int, retrn left % right;, "$. % $:");

  func(ops_->mod_, Double, Double, Double,
       retrn left - int(left / right) * right;
       , "$. - int($. / $:) * $:");

  /// =

  func(ops_->equal_, Bool, Bool, Bool, retrn left == right;, "$. == $:");

  func(ops_->equal_, Int, Int, Bool, retrn left == right;, "$. == $:");

  func(ops_->equal_, Double, Double, Bool, retrn left == right;, "$. == $:");

  /// >

  func(ops_->greater_, Int, Int, Bool, retrn left > right;, "$. > $:");

  func(ops_->greater_, Double, Double, Bool, retrn left > right;, "$. > $:");

  // <
  func(ops_->less_, Int, Int, Bool, retrn left < right;, "$. < $:");

  func(ops_->less_, Double, Double, Bool, retrn left < right;, "$. < $:");

  // >=
  func(ops_->greater_eq_, Int, Int, Bool, retrn left >= right;, "$. >= $:");

  func(ops_->greater_eq_, Double, Double, Bool, retrn left >= right;
       , "$. >= $:");

  // <=
  func(ops_->less_eq_, Int, Int, Bool, retrn left <= right;, "$. <= $:");

  func(ops_->less_eq_, Double, Double, Bool, retrn left <= right;, "$. <= $:");

  // !
  func(ops_->not_op_, Void, Bool, Bool, retrn !right;, "!$:");

  func(ops_->not_op_, Void, Int, Bool, retrn right == 0;, "$: == 0");

  func(ops_->not_op_, Void, Double, Bool, retrn right == 0;, "$: == 0");
}

void PopulateConverters() {
  /// initalizers

  func("Bool", Void, Void, Bool, retrn false;, "false");

  func("Int", Void, Void, Int, retrn 0;, "0");

  func("Double", Void, Void, Double, retrn 0.0;, "0.0");

  func("Byte", Void, Void, Byte, retrn 0;, "0");

  /// casting

  // to bool
  func("Bool", Void, Bool, Bool, retrn right, "$:");

  func("Bool", Void, Int, Bool, retrn right != 0, "($: != 0)");

  func("Bool", Void, Double, Bool, retrn right != 0, "($: != 0.0)");

  func("Bool", Bool, Void, Bool, retrn left, "$.");

  func("Bool", Int, Void, Bool, retrn left != 0, "($. != 0)");

  func("Bool", Double, Void, Bool, retrn left != 0, "($. != 0.0)");

  // to Byte
  func("Byte", Void, Bool, Byte, retrn(right ? 1 : 0), "($: ? 1 : 0)");

  func("Byte", Void, Int, Byte, retrn(unsigned char) right,
       "((unsigned char)$:)");

  func("Byte", Bool, Void, Byte, retrn(left ? 1 : 0), "($. ? 1 : 0)");

  func("Byte", Int, Void, Byte, retrn(unsigned char) left,
       "((unsigned char)$.)");

  // to Int
  func("Int", Void, Bool, Int, retrn(right ? 1 : 0), "($: ? 1 : 0)");

  func("Int", Void, Byte, Int, retrn(int) right, "((int)$:)");

  func("Int", Void, Double, Int, retrn(int) right, "((int)$:)");

  func("Int", Bool, Void, Int, retrn(left ? 1 : 0), "($. ? 1 : 0)");

  func("Int", Byte, Void, Int, retrn(int) left, "((int)$.)");

  func("Int", Double, Void, Int, retrn(int) left, "((int)$.)");

  AddAction(
      "Int", String, Void, Int,
      LAMBDA_HEADER {
        int* out = (int*)malloc(sizeof(int));
        *out = str::StringToInt(TerStr2CppStr(left_in));
        return out;
      },
      ADD_CPP_HEADER {
        AddToProgStrToInt(prog);

        prog->Name("$strToInt");
        prog->PushExpr();
        left->AddToProg(prog);
        prog->PopExpr();
      });

  // to Double
  func("Double", Void, Bool, Double, retrn(right ? 1 : 0), "($: ? 1.0 : 0.0)");

  func("Double", Void, Int, Double, retrn(double) right, "((double)$:)");

  func("Double", Bool, Void, Double, retrn(left ? 1 : 0), "($. ? 1.0 : 0.0)");

  func("Double", Int, Void, Double, retrn(double) left, "((double)$.)");

  AddAction(
      "Double", String, Void, Double,
      LAMBDA_HEADER {
        double* out = (double*)malloc(sizeof(double));
        *out = str::StringToDouble(TerStr2CppStr(left_in));
        return out;
      },
      ADD_CPP_HEADER {
        AddToProgStrToDouble(prog);

        prog->Name("$StrToDouble");
        prog->PushExpr();
        left->AddToProg(prog);
        prog->PopExpr();
      });
}

void PopulateStdFuncs() {
  // print

  func("print", Void, Void, Void, std::cout << std::endl;
       , "fputs(\"\\n\", stdout)");

  func("print", Void, Bool, Void,
       std::cout << (right ? "tru" : "fls") << std::endl;
       , "fputs($:?\"tru\\n\":\"fls\\n\", stdout)");

  func("print", Void, Byte, Void, std::cout << right << std::endl;
       , "printf(\"%c\", $:)");

  func("print", Void, Int, Void, std::cout << right << std::endl;
       , "printf(\"%d\\n\", $:)");

  func(
      "print", Void, Double, Void,
      std::cout << str::DoubleToString(right) << std::endl;
      , ADD_CPP_HEADER {
        AddToProgDoubleToStr(prog);
        AddToProgCStr(prog);

        prog->Code("printf");
        prog->PushExpr();
        prog->Code("\"%s\\n\", ");
        prog->Name("$cStr");
        prog->PushExpr();
        prog->Name("$doubleToStr");
        prog->PushExpr();
        right->AddToProg(prog);
        prog->PopExpr();
        prog->PopExpr();
        prog->PopExpr();
      });

  AddAction(
      "print", Void, String, Void,
      LAMBDA_HEADER {
        std::cout << TerStr2CppStr(right_in) << std::endl;
        return nullptr;
      },
      ADD_CPP_HEADER {
        AddToProgCStr(prog);

        prog->Code("printf");
        prog->PushExpr();
        prog->Code("\"%s\\n\", ");
        prog->Name("$cStr");
        prog->PushExpr();
        right->AddToProg(prog);
        prog->PopExpr();
        prog->PopExpr();
      });
}

void PopulateTypeInfoFuncs() {
  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make(
          [](Type left_type, Type right_type) -> Action {
            if (left_type->IsVoid() || !right_type->IsVoid()) return nullptr;

            std::string val = left_type->GetName();

            return LambdaActionT(
                left_type, right_type, String,

                [=](void* left_in, void* right_in) -> void* {
                  return CppStr2TerStr(val);
                },

                [=](Action in_left, Action in_right, CppProgram* prog) {
                  void* tb_str = CppStr2TerStr(val);
                  ConstGetActionT(tb_str, String, val, global_namespace_)
                      ->AddToProg(prog);
                  free(tb_str);
                },
                "type_name");
          }),
      "type_name");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make([](Type left_type,
                                        Type right_type) -> Action {
        if (left_type->IsVoid() || !right_type->IsVoid()) return nullptr;

        int val = left_type->GetSize();

        return LambdaActionT(
            left_type, right_type, Int,

            [=](void* left_in, void* right_in) -> void* {
              return &(*(int*)malloc(sizeof(int)) = val);
            },

            [=](Action in_left, Action in_right, CppProgram* prog) {
              ConstGetActionT(&val, Int, std::to_string(val), global_namespace_)
                  ->AddToProg(prog);
            },
            "type_size");
      }),
      "type_size");
}

void PopulateMemManagementFuncs() {
  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make(
          [](Type left_type, Type right_type) -> Action {
            if (!left_type->IsVoid() || right_type->IsVoid()) return nullptr;

            return LambdaActionT(
                left_type, right_type, right_type->GetPtr(),

                [=](void* left_in, void* right_in) -> void* {
                  size_t size = right_type->GetSize();
                  void** data_ptr = (void**)malloc(sizeof(void*));
                  *data_ptr = malloc(size);
                  memcpy(*data_ptr, right_in, size);
                  return data_ptr;
                },

                [=](Action in_left, Action in_right, CppProgram* prog) {
                  prog->Code("&");
                  prog->PushExpr();
                  std::string type_code = prog->GetTypeCode(right_type);
                  prog->Code("(*(" + type_code + "*)malloc(sizeof(" +
                             type_code + ")))");
                  prog->Code(" = ");
                  prog->PushExpr();
                  in_right->AddToProg(prog);
                  prog->PopExpr();
                  prog->PopExpr();
                },
                "new");
          }),
      "new");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make(
          [](Type left_type, Type right_type) -> Action {
            if (left_type->GetType() != TypeBase::PTR || left_type->IsVoid() ||
                !right_type->IsVoid())
              return nullptr;

            return LambdaActionT(
                left_type, right_type, left_type->GetSubType(),

                [=](void* left_in, void* right_in) -> void* {
                  size_t size = left_type->GetSubType()->GetSize();
                  void* data = malloc(size);
                  memcpy(data, *(void**)left_in, size);
                  return data;
                },

                [=](Action in_left, Action in_right, CppProgram* prog) {
                  prog->Code("*");
                  prog->PushExpr();
                  in_left->AddToProg(prog);
                  prog->PopExpr();
                },
                "dif");
          }),
      "dif");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make(
          [](Type left_type, Type right_type) -> Action {
            if (left_type->GetType() != TypeBase::PTR ||
                !right_type->Matches(left_type->GetSubType()))
              return nullptr;

            return LambdaActionT(
                left_type, right_type, Void,

                [=](void* left_in, void* right_in) -> void* {
                  memcpy(*(void**)left_in, right_in, right_type->GetSize());
                  return nullptr;
                },

                [=](Action in_left, Action in_right, CppProgram* prog) {
                  prog->Code("*");
                  prog->PushExpr();
                  in_left->AddToProg(prog);
                  prog->PopExpr();
                  prog->Code(" = ");
                  prog->PushExpr();
                  in_right->AddToProg(prog);
                  prog->PopExpr();
                },
                "dif");
          }),
      "dif");
}

void PopulateStringFuncs() {
  auto destructor_lambda = LAMBDA_HEADER {
    free(GetValFromTuple<char*>(right_in, String, "_data"));
    return nullptr;
  };

  AddAction(
      "__destroy__", Void, String, Void, destructor_lambda, ADD_CPP_HEADER {
        prog->Code("free");
        prog->PushExpr();
        right->AddToProg(prog);
        prog->Code("._data");
        prog->PopExpr();
      });

  AddAction(
      "__copy__", Void, String, String,
      LAMBDA_HEADER {
        int size = GetValFromTuple<int>(right_in, String, "_size");
        void* new_data = malloc(size);
        memcpy(new_data, GetValFromTuple<void*>(right_in, String, "_data"),
               size);
        SetValInTuple(right_in, String, "_data", new_data);
        void* out = malloc(String->GetSize());
        memcpy(out, right_in, String->GetSize());
        return out;
      },
      ADD_CPP_HEADER {
        if (!prog->HasFunc("$CopyStr")) {
          std::string str_type = prog->GetTypeCode(String);

          prog->AddFunc(
              "$CopyStr", {{str_type, "in"}}, str_type,
              "unsigned char* new_data = (unsigned char*)malloc(in._size);\n"
              "memcpy(new_data, in._data, in._size);\n"
              "in._data = new_data;\n"
              "return in;\n");
        }

        prog->Name("$copyStr");
        prog->PushExpr();
        right->AddToProg(prog);
        prog->PopExpr();
      });

  AddAction(
      "String", Void, Void, String, LAMBDA_HEADER { return CppStr2TerStr(""); },
      ADD_CPP_HEADER {
        prog->Code(prog->GetTypeCode(String) + "(0, nullptr)");
      });

  AddAction(
      "String", String, Void, String,
      LAMBDA_HEADER { return CppStr2TerStr(TerStr2CppStr(left_in)); },
      ADD_CPP_HEADER { left->AddToProg(prog); });

  AddAction(
      "String", Int, Void, String,
      LAMBDA_HEADER { return CppStr2TerStr(std::to_string(*((int*)left_in))); },
      ADD_CPP_HEADER {
        AddToProgIntToStr(prog);

        prog->Name("$intToStr");
        prog->PushExpr();
        left->AddToProg(prog);
        prog->PopExpr();
      });

  AddAction(
      "String", Double, Void, String,
      LAMBDA_HEADER {
        return CppStr2TerStr(str::DoubleToString(*((double*)left_in)));
      },
      ADD_CPP_HEADER {
        AddToProgDoubleToStr(prog);

        prog->Name("$doubleToStr");
        prog->PushExpr();
        left->AddToProg(prog);
        prog->PopExpr();
      });

  AddAction(
      "String", Bool, Void, String,
      LAMBDA_HEADER {
        if (*((bool*)left_in)) {
          return CppStr2TerStr("tru");
        } else {
          return CppStr2TerStr("fls");
        }
      },
      ADD_CPP_HEADER {
        AddToProgTbStr(prog);

        prog->Name("$pnStr");
        prog->PushExpr();
        prog->PushExpr();
        left->AddToProg(prog);
        prog->PopExpr();
        prog->Code(" ? \"tru\" : \"fls\"");
        prog->PopExpr();
      });

  AddAction(
      "len", String, Void, Int,
      LAMBDA_HEADER {
        int* out = (int*)malloc(sizeof(int));
        *out = GetValFromTuple<int>(left_in, String, "_size");
        return out;
      },
      ADD_CPP_HEADER {
        GetElemFromTupleActionT(String, "_size")
            ->AddToProg(left, void_action_, prog);
      });

  AddAction(
      "ascii", Int, Void, String,
      LAMBDA_HEADER {
        int val = *((int*)left_in);
        if (val < 0 || val >= 256) {
          throw TerebinthError(
              "tried to make ascii string out of value " + std::to_string(val),
              RUNTIME_ERROR);
        }
        std::string out;
        out += (char)val;
        return CppStr2TerStr(out);
      },
      ADD_CPP_HEADER {
        AddToProgAsciiToStr(prog);

        prog->Name("$asciiToStr");
        prog->PushExpr();
        left->AddToProg(prog);
        prog->PopExpr();
      });

  AddAction(
      "at", String, Int, Int,
      LAMBDA_HEADER {
        int index = *((int*)right_in);
        int* out = (int*)malloc(sizeof(int));
        std::string str = TerStr2CppStr(left_in);
        if (index < 0 || index >= int(str.size())) {
          throw TerebinthError("tried to access location " +
                                   std::to_string(index) + " in string " +
                                   std::to_string(str.size()) + " long",
                               RUNTIME_ERROR);
        }
        *out = str[index];
        return out;
      },
      ADD_CPP_HEADER {
        prog->Code("(int)");
        GetElemFromTupleActionT(String, "_data")
            ->AddToProg(left, void_action_, prog);
        prog->Code("[");
        prog->PushExpr();
        right->AddToProg(prog);
        prog->PopExpr();
        prog->Code("]");
      });

  AddAction(
      "sub", String,
      MakeTuple(
          std::vector<NamedType>{NamedType{"a", Int}, NamedType{"b", Int}},
          true),
      String,
      LAMBDA_HEADER {
        Type RightType = MakeTuple(
            std::vector<NamedType>{NamedType{"a", Int}, NamedType{"b", Int}},
            true);
        int start = GetValFromTuple<int>(right_in, RightType, "a");
        int end = GetValFromTuple<int>(right_in, RightType, "b");
        std::string str = TerStr2CppStr(left_in);
        if (start < 0 || end > int(str.size()) || start > end) {
          throw TerebinthError("invalid arguments sent to String.sub",
                               RUNTIME_ERROR);
        }
        return CppStr2TerStr(str.substr(start, end - start));
      },
      ADD_CPP_HEADER {
        AddToProgSubStr(prog);

        prog->Name("$subStr");
        prog->PushExpr();
        left->AddToProg(prog);
        prog->Code(", ");
        GetElemFromTupleActionT(right->GetReturnType(), "a")
            ->AddToProg(right, void_action_, prog);
        prog->Code(", ");
        GetElemFromTupleActionT(right->GetReturnType(), "b")
            ->AddToProg(right, void_action_, prog);
        prog->PopExpr();
      });

  AddAction(
      "input", String, Void, String,
      LAMBDA_HEADER {
        std::string in;
        std::cout << TerStr2CppStr(left_in);
        std::getline(std::cin, in);
        return CppStr2TerStr(in);
      },
      ADD_CPP_HEADER {
        AddToProgGetInputLine(prog);

        prog->Name("$getInputLine");
        prog->PushExpr();
        left->AddToProg(prog);
        prog->PopExpr();
      });

  AddAction(
      ops_->plus_, String, String, String,
      [=](void* left_in, void* right_in) -> void* {
        void* out =
            CppStr2TerStr(TerStr2CppStr(left_in) + TerStr2CppStr(right_in));
        return out;
      },
      ADD_CPP_HEADER {
        AddToProgConcatStr(prog);

        prog->Name("$ConcatStr");
        prog->PushExpr();
        left->AddToProg(prog);
        prog->Code(", ");
        right->AddToProg(prog);
        prog->PopExpr();
      });

  AddAction(
      ops_->equal_, String, String, Bool,
      LAMBDA_HEADER {
        bool* out = (bool*)malloc(sizeof(bool));
        *out = TerStr2CppStr(left_in) == TerStr2CppStr(right_in);
        return out;
      },
      ADD_CPP_HEADER {
        AddToProgEqStr(prog);

        prog->Name("$eqStr");
        prog->PushExpr();
        left->AddToProg(prog);
        prog->Code(", ");
        right->AddToProg(prog);
        prog->PopExpr();
      });
}

void PopulateArrayFuncs() {
  TupleTypeMaker maker;
  maker.Add("_size", Int);
  maker.Add("_capacity", Int);
  maker.Add("_data", Whatev->GetPtr());
  Array = maker.Get(false);

  AddType(Array, "Array");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make(
          [](Type left_type, Type right_type) -> Action {
            assert left_type->IsCreatable() && right_type->IsVoid()
                                                   otherwise return nullptr;

            Type contentsType = left_type;
            Type array_type =
                Array
                    ->ActuallyIs(MakeTuple(
                        {{"a", Int}, {"b", Int}, {"c", contentsType->GetPtr()}},
                        true))
                    ->GetPtr();

            return LambdaActionT(
                left_type, right_type, array_type,

                [=](void* left_in, void* right_in) -> void* {
                  void* out = malloc(array_type->GetSize());
                  SetValInTuple(out, array_type, "_size", 0);
                  SetValInTuple(out, array_type, "_capacity", 0);
                  SetValInTuple<void*>(out, array_type, "_data", nullptr);
                  return out;
                },

                [=](Action in_left, Action in_right, CppProgram* prog) {
                  throw TerebinthError("not yet implemented", INTERNAL_ERROR);
                },

                "Array");
          }),
      "Array");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make([](Type left_type,
                                        Type right_type) -> Action {
        assert left_type->Matches(Array) otherwise return nullptr;

        Type array_type = Array->ActuallyIs(left_type->GetPtr());
        Type contentsType = array_type->GetSubType("_data").type->GetSubType();

        assert right_type->Matches(contentsType) otherwise return nullptr;

        return LambdaActionT(
            left_type, right_type, Void,

            [=](void* left_in, void* right_in) -> void* {
              int size = GetValFromTuple<int>(left_in, array_type, "_size");
              int capacity =
                  GetValFromTuple<int>(left_in, array_type, "_capacity");
              void* data = GetValFromTuple<void*>(left_in, array_type, "_data");

              if (size + 1 > capacity) {
                if (capacity < 1000)
                  capacity = 1000;
                else
                  capacity *= 2;

                SetValInTuple<int>(left_in, array_type, "_capacity", capacity);
                void* new_data = malloc(capacity * contentsType->GetSize());
                memcpy(new_data, data, size * contentsType->GetSize());
                free(data);
                data = new_data;
                SetValInTuple<void*>(left_in, array_type, "_data", data);
              }

              SetValInTuple<int>(left_in, array_type, "_size", size + 1);
              memcpy((char*)data + size * contentsType->GetSize(), right_in,
                     contentsType->GetSize());

              return nullptr;
            },

            [=](Action in_left, Action in_right, CppProgram* prog) {
              throw TerebinthError("not yet implemented", INTERNAL_ERROR);
            },
            "append");
      }),
      "append");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make([](Type left_type,
                                        Type right_type) -> Action {
        assert left_type->Matches(Array->GetPtr()) &&
            right_type->Matches(Int) otherwise return nullptr;

        Type array_type = Array->ActuallyIs(left_type->GetSubType());
        Type contentsType = array_type->GetSubType("_data").type->GetSubType();
        size_t elemSize = contentsType->GetSize();

        return LambdaActionT(
            left_type, right_type, contentsType,

            [=](void* left_in, void* right_in) -> void* {
              int size = GetValFromTuple<int>(left_in, array_type, "_size");
              if (*(int*)right_in < 0 || *(int*)right_in >= size)
                throw TerebinthError(
                    "index out of bounds, tried to get element at position " +
                        std::to_string(*(int*)right_in) + " in array " +
                        std::to_string(size) + " long",
                    RUNTIME_ERROR);

              void* out = malloc(contentsType->GetSize());
              memcpy(out,
                     GetValFromTuple<char*>(left_in, array_type, "_data") +
                         (*(int*)right_in) * elemSize,
                     elemSize);
              return out;
            },

            [=](Action in_left, Action in_right, CppProgram* prog) {
              throw TerebinthError("not yet implemented", INTERNAL_ERROR);
            },
            "get");
      }),
      "get");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make([](Type left_type,
                                        Type right_type) -> Action {
        Type array_type = Array->ActuallyIs(left_type->GetSubType());
        Type contentsType = array_type->GetSubType("_data").type->GetSubType();

        Type inputType =
            MakeTuple({{"index", Int}, {"value", contentsType}}, true);

        assert left_type->GetSubType()->Matches(Array) &&
            right_type->Matches(inputType) otherwise return nullptr;

        size_t elemSize = contentsType->GetSize();

        return LambdaActionT(
            left_type, right_type, contentsType,

            [=](void* left_in, void* right_in) -> void* {
              int size = GetValFromTuple<int>(left_in, array_type, "_size");
              int index = GetValFromTuple<int>(right_in, inputType, "index");

              if (index < 0 || index >= size)
                throw TerebinthError(
                    "index out of bounds, tried to set element at position " +
                        std::to_string(index) + " in array " +
                        std::to_string(size) + " long",
                    RUNTIME_ERROR);

              char* data = GetValFromTuple<char*>(left_in, array_type, "_data");

              memcpy(data + index * elemSize,
                     (char*)right_in + inputType->GetSubType("value").offset,
                     contentsType->GetSize());

              return nullptr;
            },

            [=](Action in_left, Action in_right, CppProgram* prog) {
              throw TerebinthError("not yet implemented", INTERNAL_ERROR);
            },
            "set");
      }),
      "set");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make([](Type left_type,
                                        Type right_type) -> Action {
        assert left_type->Matches(Array) && right_type->IsVoid()
                                                otherwise return nullptr;

        Type array_type = Array->ActuallyIs(left_type);
        Type contentsType = array_type->GetSubType("_data").type->GetSubType();

        return LambdaActionT(
            left_type, right_type, Int,

            [=](void* left_in, void* right_in) -> void* {
              int* out = (int*)malloc(sizeof(int));
              *out = GetValFromTuple<int>(left_in, array_type, "_size");
              return out;
            },

            [=](Action in_left, Action in_right, CppProgram* prog) {
              throw TerebinthError("not yet implemented", INTERNAL_ERROR);
            },
            "len");
      }),
      "len");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make([](Type left_type,
                                        Type right_type) -> Action {
        assert left_type->IsVoid() && right_type->Matches(Array)
                                          otherwise return nullptr;

        Type array_type = Array->ActuallyIs(right_type);
        Type contentsType = array_type->GetSubType("_data").type->GetSubType();
        size_t elemSize = contentsType->GetSize();

        return LambdaActionT(
            Void, right_type, right_type,

            [=](void* left_in, void* right_in) -> void* {
              // error.log("Array destroyer called", JSYK);
              int size = GetValFromTuple<int>(right_in, array_type, "_size");
              void* new_data = malloc(elemSize * size);
              void* old_data =
                  GetValFromTuple<void*>(right_in, array_type, "_data");
              memcpy(new_data, old_data, elemSize * size);
              SetValInTuple(right_in, array_type, "_data", new_data);
              void* out = malloc(array_type->GetSize());
              memcpy(out, right_in, array_type->GetSize());
              return out;
            },

            [=](Action in_left, Action in_right, CppProgram* prog) {
              throw TerebinthError("not yet implemented", INTERNAL_ERROR);
            },
            "__copy__");
      }),
      "__copy__");

  global_namespace_->AddNode(
      AstWhatevToActionFactory::Make(
          [](Type left_type, Type right_type) -> Action {
            assert left_type->IsVoid() && right_type->Matches(Array)
                                              otherwise return nullptr;

            Type array_type = Array->ActuallyIs(right_type);

            return LambdaActionT(
                Void, right_type, Void,

                [=](void* left_in, void* right_in) -> void* {
                  free(GetValFromTuple<void*>(right_in, array_type, "_data"));
                  return nullptr;
                },

                [=](Action in_left, Action in_right, CppProgram* prog) {
                  throw TerebinthError("not yet implemented", INTERNAL_ERROR);
                },
                "__destroy__");
          }),
      "__destroy__");
}

void PopulateIntArrayAndFuncs() {
  TupleTypeMaker maker;
  maker.Add("_size", Int);
  maker.Add("_data", Int->GetPtr());
  IntArray = maker.Get(false);

  AddType(IntArray, "IntArray");

  AddAction(
      "IntArray", Void, Int, IntArray,
      LAMBDA_HEADER {
        char* out = (char*)malloc(IntArray->GetSize());
        int size_in = *(int*)right_in;
        int* int_ptr_data = (int*)malloc(Int->GetSize() * size_in);
        SetValInTuple<int>(out, IntArray, "_size", size_in);
        SetValInTuple<int*>(out, IntArray, "_data", int_ptr_data);
        return out;
      },
      ADD_CPP_HEADER {
        AddToProgMakeIntArray(prog);
        prog->Name("$MakeIntArray");
        prog->PushExpr();
        right->AddToProg(prog);
        prog->PopExpr();
      });

  AddAction(
      "len", IntArray, Void, Int,
      LAMBDA_HEADER {
        int* out = (int*)malloc(sizeof(int));
        *out = GetValFromTuple<int>(left_in, IntArray, "_size");
        return out;
      },
      ADD_CPP_HEADER {
        GetElemFromTupleActionT(IntArray, "_size")
            ->AddToProg(left, void_action_, prog);
      });

  AddAction(
      "get", IntArray, Int, Int,
      LAMBDA_HEADER {
        int pos = *(int*)right_in;
        int size = GetValFromTuple<int>(left_in, IntArray, "_size");
        if (pos < 0 || pos >= size)
          throw TerebinthError("tried to acces position " +
                                   std::to_string(pos) + " of array " +
                                   std::to_string(size) + " long",
                               RUNTIME_ERROR);
        int* array_ptr = GetValFromTuple<int*>(left_in, IntArray, "_data");
        int val = *(array_ptr + pos);
        int* out = (int*)malloc(sizeof(int));
        *out = val;
        return out;
      },
      ADD_CPP_HEADER {
        GetElemFromTupleActionT(IntArray, "_data")
            ->AddToProg(left, void_action_, prog);
        prog->Code("[");
        prog->PushExpr();
        right->AddToProg(prog);
        prog->PopExpr();
        prog->Code("]");
      });

  AddAction(
      "set", IntArray, TER_Tuple(Int, Int), Void,
      LAMBDA_HEADER {
        Type right_type = TER_Tuple(Int, Int);
        int pos = GetValFromTuple<int>(right_in, right_type, "a");
        int size = GetValFromTuple<int>(left_in, IntArray, "_size");
        if (pos < 0 || pos >= size)
          throw TerebinthError("tried to set value at position " +
                                   std::to_string(pos) + " of array " +
                                   std::to_string(size) + " long",
                               RUNTIME_ERROR);
        int val = GetValFromTuple<int>(right_in, right_type, "b");
        int* array_ptr = GetValFromTuple<int*>(left_in, IntArray, "_data");
        *(array_ptr + pos) = val;
        return nullptr;
      },
      ADD_CPP_HEADER {
        GetElemFromTupleActionT(IntArray, "_data")
            ->AddToProg(left, void_action_, prog);
        prog->Code("[");
        prog->PushExpr();
        GetElemFromTupleActionT(right->GetReturnType(), "a")
            ->AddToProg(right, void_action_, prog);
        prog->PopExpr();
        prog->Code("]");
        prog->Code(" = ");
        GetElemFromTupleActionT(right->GetReturnType(), "b")
            ->AddToProg(right, void_action_, prog);
      });
}

void PopulateNonStdFuncs() {
  func("printc", Void, Int, Void, putchar((char)right), "putchar($:)");

  func("inputc", Void, Void, Int, retrn getchar(), "getchar()");

  func(
      "inputInt", Void, Void, Int, int val; std::cin >> val; retrn val;
      , ADD_CPP_HEADER {
        if (!prog->HasFunc("-GetIntInput")) {
          prog->PushFunc("-GetIntInput", Void, Void, Int);
          prog->DeclareVar("tmp", Int);
          prog->Code("scanf");
          prog->PushExpr();
          prog->Code("\"%d\", &");
          prog->Name("tmp");
          prog->PopExpr();
          prog->Endln();
          prog->Code("return ");
          prog->Name("tmp");
          prog->Endln();
          prog->PopFunc();
        }

        prog->Name("-GetIntInput");
        prog->Code("()");
      });

  AddAction(
      "runCmd", Void, String, String,
      LAMBDA_HEADER {
        return CppStr2TerStr(str::RunCmd(TerStr2CppStr(right_in)));
      },
      ADD_CPP_HEADER {
        AddToProgRunCmd(prog);
        prog->Name("$RunCmd");
        prog->PushExpr();
        right->AddToProg(prog);
        prog->PopExpr();
      });
}

void PopulateCppInterfaceFuncs() {
  AddAction(
      "cpp_code", Void, String, Void,
      LAMBDA_HEADER {
        throw TerebinthError(
            "you can't run interpreter with code that uses 'cpp_code'",
            SOURCE_ERROR);
      },
      ADD_CPP_HEADER {
        prog->PushBlock();
        AddToProgStrWithEscapedNames(
            prog, TerStr2CppStr(right->Execute(nullptr, nullptr)));
        prog->PopBlock();
      });

  AddAction(
      "cpp_head", Void, String, Void,
      LAMBDA_HEADER {
        throw TerebinthError(
            "you can't run interpreter with code that uses 'cpp_head'",
            SOURCE_ERROR);
      },
      ADD_CPP_HEADER {
        prog->AddHeadCode(TerStr2CppStr(right->Execute(nullptr, nullptr)));
      });
}

void PopulateTerebinthStdLib() {
  BasicSetup();
  PopulateBasicTypes();
  PopulateConstants();
  PopulateOperators();
  PopulateConverters();
  PopulateStdFuncs();
  PopulateTypeInfoFuncs();
  PopulateMemManagementFuncs();
  PopulateStringFuncs();
  PopulateIntArrayAndFuncs();
  PopulateArrayFuncs();
  PopulateNonStdFuncs();
  PopulateCppInterfaceFuncs();
}
