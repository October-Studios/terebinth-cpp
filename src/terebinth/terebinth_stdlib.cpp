#include <ios>
#include "ast_node.h"
#include "error_handler.h"
#include "terebinth_program.h"
#include "all_operators.h"
#include "stack_frame.h"
#include "namespace.h"
#include "cpp_program.h"
#include "string_utils.h"
#include "type.h"

#define CONCAT(a, b) a##_##b
#define GET_TYPES_Tuple(t0, t1) t0, t1

#define assert if (
#define otherwise ) {} else

#define GetTbthType(type_in) CONCAT(TBTH, type_in)
#define GetCppType(type_in) CONCAT(CPP, type_in)

#define CPP_Double double
#define CPP_Byte unsigned char
#define CPP_Int int
#define CPP_Bool bool
#define CPP_Void char

#define TBTH_String String
#define TBTH_Double Double
#define TBTH_Int Int
#define TBTH_Byte Byte
#define TBTH_Bool Bool
#define TBTH_Void Void

#define TBTH_Tuple(t1, t2) MakeTuple(std::vector<NamedType>({{"a", t1}, {"b", t2}}), true)

#define LAMBDA_HEADER [](void* left_in, void* right_in)->void*
#define ADD_CPP_HEADER [](Action left, Action right, CppProgram* prog)->void

#define retrn out =
#define GET_PTR_VAL(type_in, var_in_name) = *((GetCppType(type_in)*)(var_in_name))

#define DO_INSTANTIATE(type_in, var_out_name, val_in) GetCppType(type_in) var_out_name val_in;
#define DONT_INSTANTIATE(type_in, var_out_name, val_in) ;
#define INSTANTIATE_CPP_TUPLE(t0, t1, var_out_name, val_in)\
  DO_INSTANTIATE(t0, CONCAT(var_out_name, 0), val_in)\
  DO_INSTANTIATE(t1, CONCAT(var_out_name, 1), ((char*)val_in) + sizeof(GetCppType(t0)))

#define DO_RETURN_VAL(type_in, var_name) void* out_ptr = malloc(GetTbthType(type_in)->GetSize()); memcpy(out_ptr, &var_name, GetTbthType(type_in)->GetSize()); return out_ptr;
#define DONT_RETURN_VAL(type_in, var_name) return nullptr;

#define INSTANTIATE_String DO_INSTANTIATE
#define INSTANTIATE_Double DO_INSTANTIATE
#define INSTANTIATE_Int DO_INSTANTIATE
#define INSTANTIATE_Byte DO_INSTANTIATE
#define INSTANTIATE_Bool DO_INSTANTIATE
#define INSTANTIATE_Void DONT_INSTANTIATE
#define INSTANTIATE_Tuple__(type_in, var_out_name, val_in) INSTANTIATE_CPP_TUPLE(type_in, var_out_name, val_in)
#define INSTANTIATE_Tuple_(type_in, var_out_name, val_in) INSTANTIATE_Tuple__(GET_TYPES##_##type_in, var_out_name, val_in)
#define INSTANTIATE_Tuple(t1, t2) INSTANTIATE_Tuple_

#define RETURN_Double DO_RETURN_VAL
#define RETURN_Int DO_RETURN_VAL
#define RETURN_Byte DO_RETURN_VAL
#define RETURN_Bool DO_RETURN_VAL
#define RETURN_Void DONT_RETURN_VAL

#define func(name_text, left_type, right_type, return_type, lambda_body, cpp) \
AddAction(name_text, GetTbthType(left_type), GetTbthType(right_type), GetTbthType(return_type), LAMBDA_HEADER \
{  \
  INSTANTIATE##_##left_type(left_type, left, GET_PTR_VAL(left_type, left_in)) \
  INSTANTIATE##_##right_type(right_type, right, GET_PTR_VAL(right_type, right_in)) \
  INSTANTIATE##_##return_type(return_type, out, ;) \
  lambda_body; \
  CONCAT(RETURN, return_type)(return_type, out) \
}, \
cpp \
)

Action void_action_;

Namespace global_namespace_;
Namespace table;

std::string GetText(Operator op) { return op->GetText(); }
std::string GetText(std::string in) { return in; }

void AddConst(void* data, Type type, std::string name) {
  global_namespace_->AddNode(AstActionWrapper::Make(ConstGetAction(data, type, name, global_namespace_)), name);
}

template<typename T>
void AddAction(T id, Type left_type, Type right_type, Type return_type, std::function<
    void*(void*, void*)> lambda, std::function<void(Action in_left, Action in_right, CppProgram* prog)> cpp_writer) {
  global_namespace_->AddNode(AstActionWrapper::Make(LambdaAction(left_type,
      right_type, return_type, lambda, cpp_writer, GetText(id))), GetText(id));
}

void AddType(Type type, std::string id) {
  auto node = AstTypeType::Make(type);
  node->SetInput(global_namespace_, false, Void, Void);
  global_namespace_->AddNode(std::move(node), id);
}

std::function<void(Action in_left, Action in_right, CppProgram* prog)> StringToLambda(std::string cpp_code) {
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
          throw TerebinthError("invalid '$' escape in C++ code: " + cpp_code, INTERNAL_ERROR);
        }
      }
    } while (i >= 0);
  };
}

void PopulateCppInterfaceFuncs() {
  AddAction("cpp_code", Void, String, Void, LAMBDA_HEADER
    {
      throw TerebinthError("you can't run interpreter with code that uses 'cpp_code'", SOURCE_ERROR);
    },
    ADD_CPP_HEADER
    {
      prog->PushBlock();
      AddToProgStrWithEscapedNames(prog, TbthStr2CppStr(right->Execute(nullptr, nullptr)));
      prog->PopBlock();
    }
  );

  AddAction("cpp_head", Void, String, Void, LAMBDA_HEADER
    {
      throw TerebinthError("you can't run interpreter with code that uses 'cpp_head'", SOURCE_ERROR);
    },
    ADD_CPP_HEADER
    {
      prog->AddHeadCode(TbthStr2CppStr(right->Execute(nullptr, nullptr)));
    }
  );
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
