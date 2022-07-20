#include "type.h"

#include <sstream>

#include "cpp_program.h"
#include "error_handler.h"
#include "util/string_utils.h"

class VoidType : public TypeBase {
 public:
  virtual auto GetString() -> std::string { return "VOID"; }

  auto GetCompactString() -> std::string { return "v"; }

  auto GetCppLiteral(void *data, CppProgram *prog) -> std::string {
    throw TerebinthError("tried to get the literal value of 'void;",
                         INTERNAL_ERROR);
  }

  auto IsCreatable() -> bool { return false; }

  auto IsVoid() -> bool { return true; }

  auto GetSize() -> size_t { return 0; }

  auto GetType() -> PrimitiveType { return VOID; };

 protected:
  auto MatchesSameTypeType(Type other) -> bool { return true; }
};

class UnknownType : public TypeBase {
 public:
  virtual auto GetString() -> std::string { return "UNKNOWN"; }

  auto GetCompactString() -> std::string { return "u"; }

  auto GetCppLiteral(void *data, CppProgram *prog) -> std::string {
    return "/* can not instantiate unknown type */";
  }

  auto IsCreatable() -> bool { return false; }

  auto IsVoid() -> bool { return false; }

  auto GetSize() -> size_t { return 0; }

  auto GetType() -> PrimitiveType { return UNKNOWN; };

 protected:
  auto MatchesSameTypeType(Type other) -> bool { return false; }
};

class PrimType : public TypeBase {
 public:
    switch (primType) {
        return "b";
      case BYTE:
        return "y";
      case INT:
        return "i";
      case DOUBLE:
        return "d";

      default:
        throw TerebinthError("tried to make " + GetString() + " compact",
                             INTERNAL_ERROR);
    }
  }

  auto GetString() -> std::string { return TypeBase::GetString(primType); }

  auto GetCppLiteral(void *data, CppProgram *prog) -> std::string {
    std::string val;

    switch (primType) {
      case BOOL:
        val = (*(bool *)data) ? "true" : "false";
        break;
      case BYTE:
        val = std::to_string(*(unsigned char *)data);
        break;
      case INT:
        val = std::to_string(*(int *)data);
        break;
      case DOUBLE:
        val = str::DoubleToString(*(double *)data);
        break;
        /*{
          std::ostringstream ss;
          ss << *(double*)data;
          val = ss.str();
        }
        break;*/

      default:
        throw TerebinthError("tried to convert " + GetString() + " to C++ code",
                             INTERNAL_ERROR);
    }

    return val;
  }

  auto GetSize() -> size_t {
    switch (primType) {
      case BOOL:
        return sizeof(bool);
      case BYTE:
        return sizeof(unsigned char);
      case INT:
        return sizeof(int);
      case DOUBLE:
        return sizeof(double);

      default:
        throw TerebinthError("tried to get size of " + GetString(),
                             INTERNAL_ERROR);
    }
  }

  auto GetType() -> PrimitiveType { return primType; }

 protected:
  PrimitiveType primType;

  auto MatchesSameTypeType(Type other) -> bool {
    return other->GetType() == primType;
  }
};

class TupleType : public TypeBase {
 public:
  explicit TupleType(std::unique_ptr<std::vector<NamedType>> in,
                     bool isAnonymousIn) {
    if (in == nullptr)
      error_.Log(FUNC +
                     " sent null input, compiler will likely shit itself in "
                     "the near future",
                 INTERNAL_ERROR);

    sub_types_ = std::move(in);

    for (auto i : *sub_types_) {
      if (i.type->IsWhatev()) {
        hasWhatev = true;
      }

      if (!i.type->IsCreatable()) {
        hasOnlyCreatable = false;
      }
    }

    isAnonymous = isAnonymousIn;
  }

  ~TupleType() = default;

  auto GetString() -> std::string {
    std::string out;

    out += "{";

    for (int i = 0; i < int(sub_types_->size()); i++) {
      if (i) out += ", ";

      out += (*sub_types_)[i].name + ": " + (*sub_types_)[i].type->GetString();
    }

    out += "} (tuple)";

    return out;
  }

  auto GetCompactString() -> std::string {
    std::string out;

    out += "Tt_";

    for (int i = 0; i < int(sub_types_->size()); i++) {
      // out+=(*sub_types_)[i].name.size()+"_"+(*sub_types_)[i].name+"_"+(*sub_types_)[i].type->GetCompactString()+"_";
      out += (*sub_types_)[i].type->GetCompactString() + "_";
    }

    out += "tT";

    return out;
  }

  auto GetCppLiteral(void *data, CppProgram *prog) -> std::string {
    if (sub_types_->size() == 1) {
      return (*sub_types_)[0].type->GetCppLiteral(data, prog);
    }

    std::string out;
    out += prog->GetTypeCode(shared_from_this());

    out += "(";

    for (int i = 0; i < int(sub_types_->size()); i++) {
      if (i) out += ", ";

      out += (*sub_types_)[i].type->GetCppLiteral(
          (char *)data + GetSubType((*sub_types_)[i].name).offset, prog);
    }

    out += ")";

    return out;
  }

  auto GetSize() -> size_t {
    size_t sum = 0;

    for (auto i : *sub_types_) {
      sum += i.type->GetSize();
    }

    return sum;
  }

  auto GetType() -> PrimitiveType { return TUPLE; }

  auto GetSubType(std::string name) -> OffsetAndType {
    size_t offset = 0;

    for (auto i : *sub_types_) {
      if (i.name == name)
        return {offset, i.type};
      else
        offset += i.type->GetSize();
    }

    return {0, nullptr};
  }

  auto *GetAllSubTypes() -> std::vector<NamedType> { return &(*sub_types_); }

  auto IsWhatev() -> bool { return hasWhatev; }

  auto IsCreatable() -> bool { return hasOnlyCreatable; }

  auto ActuallyIs(Type target) -> Type {
    if (target->GetType() != TUPLE) {
      if ((*sub_types_).size() == 1) {
        return (*sub_types_)[0].type->ActuallyIs(target);
      } else {
        throw TerebinthError(
            "ActuallyIs called on tuple with target that is not tuple",
            INTERNAL_ERROR);
      }
    }

    TupleTypeMaker maker;

    auto targetTypes = target->GetAllSubTypes();

    for (int i = 0; i < (int)sub_types_->size(); i++) {
      maker.Add((*sub_types_)[i].name,
                (*sub_types_)[i].type->ActuallyIs((*targetTypes)[i].type));
    }

    return maker.Get(true);
  }

 protected:
  auto MatchesSameTypeType(Type other) -> bool {
    auto o = (TupleType *)(&(*other));

    if (!isAnonymous && !o->isAnonymous) return false;

    if (sub_types_->size() != o->sub_types_->size()) return false;

    for (int i = 0; i < int(sub_types_->size()); i++) {
      // if ((*sub_types_)[i].name!=(*o->sub_types_)[i].name)
      //  return false;

      // if ((*sub_types_)[i].type!=(*o->sub_types_)[i].type)
      //  return false;

      if (!(*sub_types_)[i].type->Matches((*o->sub_types_)[i].type))
        return false;
    }

    return true;
  }

 private:
  std::unique_ptr<std::vector<NamedType>> sub_types_;
  bool isAnonymous = true;
  bool hasWhatev = false;
  bool hasOnlyCreatable = true;
};

class PtrType : public TypeBase {
 public:
  explicit PtrType(Type in) { type = in; }

  auto GetString() -> std::string {
    return "-> " + type->GetString() + " (pointer)";
  }

  auto GetCompactString() -> std::string {
    return "Pp_" + type->GetCompactString() + "_pP";
  }

  auto GetCppLiteral(void *data, CppProgram *prog) -> std::string {
    std::string name = std::to_string((long long)data);
    prog->DeclareGlobal(name, type, type->GetCppLiteral(*(void **)data, prog));
    std::string out = "&" + prog->GetGlobalNames()->GetCpp(name);
    return out;
  }

  auto GetSize() -> size_t { return sizeof(void *); }

  auto GetType() -> PrimitiveType { return PTR; }

  auto GetSubType() -> Type { return type; }

  auto ActuallyIs(Type target) -> Type {
    return GetSubType()->ActuallyIs(target->GetSubType())->GetPtr();
  }

 protected:
  Type type;

  auto MatchesSameTypeType(Type other) -> bool {
    return ((PtrType *)(&(*other)))->type->Matches(type);
  }
};

class MetaType : public TypeBase {
 public:
  explicit MetaType(Type in) { type = in; }

  auto GetString() -> std::string {
    return "{" + type->GetString() + "} (meta type)";
  }

  auto GetCompactString() -> std::string {
    return "Mm_" + type->GetCompactString() + "_mM";
  }

  auto GetCppLiteral(void *data, CppProgram *prog) -> std::string {
    return "/* can't add meta type to C++ code */";
  }

  auto GetSize() -> size_t { return 0; }

  auto IsCreatable() -> bool { return false; }

  auto GetType() -> PrimitiveType { return METATYPE; }

  auto GetSubType() -> Type { return type; }

  auto IsWhatev() -> bool { return type->IsWhatev(); }

  auto ActuallyIs(Type target) -> Type {
    return GetSubType()->ActuallyIs(target->GetSubType())->GetMeta();
  }

 protected:
  Type type;

  auto MatchesSameTypeType(Type other) -> bool {
    return ((MetaType *)(&(*other)))->type == type;
  }
};

class WhatevType : public TypeBase {
 public:
  WhatevType() = default;

  auto GetString() -> std::string { return "(whatev type)"; }

  auto GetCompactString() -> std::string { return "W"; }

  auto IsCreatable() -> bool { return false; }

  auto GetCppLiteral(void *data, CppProgram *prog) -> std::string {
    throw TerebinthError(
        "GetCppLiteral called on whatev type, wich should not have happened",
        INTERNAL_ERROR);
  }

  auto GetSize() -> size_t {
    throw TerebinthError(
        "GetSize called on whatev type, wich should not have happened",
        INTERNAL_ERROR);
  }

  auto GetType() -> PrimitiveType { return WHATEV; }

  auto IsWhatev() -> bool { return true; }

  auto GetSubType() -> Type {
    throw TerebinthError(
        "GetSubType called on whatev type, wich should not have happened",
        INTERNAL_ERROR);
  }

  auto ActuallyIs(Type target) -> Type { return target; }

 protected:
  Type type;

  auto MatchesSameTypeType(Type other) -> bool { return true; }
};

Type TypeBase::MakeNewVoid() { return Type(new VoidType); }

Type TypeBase::MakeNewPrimitive(PrimitiveType typeIn) {
  return Type(new PrimType(typeIn));
}

Type TypeBase::MakeNewWhatev() { return Type(new WhatevType()); }

std::vector<NamedType> *TypeBase::GetAllSubTypes() {
  throw TerebinthError("GetAllSubTypes called on type that was not a tuple",
                       INTERNAL_ERROR);
}

const Type Unknown(new UnknownType);
const Type Void = TypeBase::MakeNewVoid();
const Type Whatev = TypeBase::MakeNewWhatev();
const Type Bool = TypeBase::MakeNewPrimitive(TypeBase::BOOL);
const Type Byte = TypeBase::MakeNewPrimitive(TypeBase::BYTE);
const Type Int = TypeBase::MakeNewPrimitive(TypeBase::INT);
const Type Double = TypeBase::MakeNewPrimitive(TypeBase::DOUBLE);
Type String = nullptr;

Type TypeBase::GetMeta() { return Type(new MetaType(shared_from_this())); }

Type TypeBase::GetPtr() {
  if (!PtrToMe) PtrToMe = Type(new PtrType(shared_from_this()));

  return PtrToMe;
}

std::string TypeBase::GetString(PrimitiveType in) {
  switch (in) {
    case UNKNOWN:
      return "UNKNOWN_TYPE";
    case VOID:
      return "VOID";
    case BOOL:
      return "BOOL";
    case BYTE:
      return "BYTE";
    case INT:
      return "INT";
    case DOUBLE:
      return "DOUBLE";
    case TUPLE:
      return "TUPLE";
    case METATYPE:
      return "METATYPE";
    default:
      return "ERROR_GETTING_TYPE";
  }
}

bool TypeBase::Matches(Type other) {
  if (other == shared_from_this()) return true;

  if (GetType() == WHATEV || other->GetType() == WHATEV) return true;

  if (GetType() == TUPLE) {
    auto sub_types_ = GetAllSubTypes();

    if (sub_types_->size() == 1) {
      if ((*sub_types_)[0].type->Matches(other)) return true;
    }
  }

  if (other->GetType() == TUPLE) {
    auto sub_types_ = other->GetAllSubTypes();

    if (sub_types_->size() == 1) {
      if ((*sub_types_)[0].type->Matches(shared_from_this())) return true;
    }
  }

  if (other->GetType() != GetType()) return false;

  return MatchesSameTypeType(other);
}

Type TypeBase::ActuallyIs(Type target) {
  if (!Matches(target)) {
    throw TerebinthError("ActuallyIs called with type that doesn't match",
                         INTERNAL_ERROR);
  }

  if (IsWhatev()) {
    throw TerebinthError("ActuallyIs not implemented properly for Whatev type",
                         INTERNAL_ERROR);
  }

  return shared_from_this();
}

auto MakeTuple(const std::vector<NamedType> &in, bool isAnonymous) -> Type {
  auto ptr =
      std::unique_ptr<std::vector<NamedType>>(new std::vector<NamedType>(in));
  return Type(new TupleType(move(ptr), isAnonymous));
}

TupleTypeMaker::TupleTypeMaker() {
  sub_types_ =
      std::unique_ptr<std::vector<NamedType>>(new std::vector<NamedType>);
}

void TupleTypeMaker::Add(std::string name, Type type) {
  if (sub_types_)
    sub_types_->push_back(NamedType{name, type});
  else
    error_.Log(FUNC + "called after type has been created", INTERNAL_ERROR);
}

void TupleTypeMaker::Add(Type type) {
  std::string name = GetUniqueName();

  if (!name.empty()) {
    Add(name, type);
  } else {
    error_.Log("finding unique name in tuple type failed", INTERNAL_ERROR);
  }
}

Type TupleTypeMaker::Get(bool isAnonymous) {
  if (sub_types_)
    return Type(new TupleType(std::move(sub_types_), isAnonymous));
  else
    error_.Log(FUNC + "called after type has been created", INTERNAL_ERROR);

  return Void;
}

std::string TupleTypeMaker::GetUniqueName() {
  for (char c = 'a'; c <= 'z'; ++c) {
    std::string str;
    str += c;
    bool valid = true;

    for (auto i : *sub_types_) {
      if (i.name == str) {
        valid = false;
        break;
      }
    }

    if (valid) return str;
  }

  error_.Log("you've gotta be fuckin kidding", SOURCE_ERROR);
  return "";
}
