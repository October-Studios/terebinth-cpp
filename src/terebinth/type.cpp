#include "type.h"

#include <sstream>

#include "cpp_program.h"
#include "error_handler.h"
#include "util/string_utils.h"

class VoidType : public TypeBase {
public:
  virtual std::string GetString() { return "VOID"; }

  std::string GetCompactString() { return "v"; }

  std::string GetCppLiteral(void *data, CppProgram *prog) {
    throw TerebinthError("tried to get the literal value of 'void;",
                         INTERNAL_ERROR);
  }

  bool IsCreatable() { return false; }

  bool IsVoid() { return true; }

  size_t GetSize() { return 0; }

  PrimitiveType GetType() { return VOID; };

protected:
  bool MatchesSameTypeType(Type other) { return true; }
};

class UnknownType : public TypeBase {
public:
  virtual std::string GetString() { return "UNKNOWN"; }

  std::string GetCompactString() { return "u"; }

  std::string GetCppLiteral(void *data, CppProgram *prog) {
    return "/* can not instantiate unknown type */";
  }

  bool IsCreatable() { return false; }

  bool IsVoid() { return false; }

  size_t GetSize() { return 0; }

  PrimitiveType GetType() { return UNKNOWN; };

protected:
  bool MatchesSameTypeType(Type other) { return false; }
};

class PrimType : public TypeBase {
public:
  PrimType(PrimitiveType in) { primType = in; }

  std::string GetCompactString() {
    switch (primType) {
    case BOOL:
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

  std::string GetString() { return TypeBase::GetString(primType); }

  std::string GetCppLiteral(void *data, CppProgram *prog) {
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

  size_t GetSize() {
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

  PrimitiveType GetType() { return primType; }

protected:
  PrimitiveType primType;

  bool MatchesSameTypeType(Type other) { return other->GetType() == primType; }
};

class TupleType : public TypeBase {
public:
  TupleType(std::unique_ptr<std::vector<NamedType>> in, bool isAnonymousIn) {
    if (in == nullptr)
      error_.Log(FUNC + " sent null input, compiler will likely shit itself in "
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

  ~TupleType() {}

  std::string GetString() {
    std::string out;

    out += "{";

    for (int i = 0; i < int(sub_types_->size()); i++) {
      if (i)
        out += ", ";

      out += (*sub_types_)[i].name + ": " + (*sub_types_)[i].type->GetString();
    }

    out += "} (tuple)";

    return out;
  }

  std::string GetCompactString() {
    std::string out;

    out += "Tt_";

    for (int i = 0; i < int(sub_types_->size()); i++) {
      // out+=(*sub_types_)[i].name.size()+"_"+(*sub_types_)[i].name+"_"+(*sub_types_)[i].type->GetCompactString()+"_";
      out += (*sub_types_)[i].type->GetCompactString() + "_";
    }

    out += "tT";

    return out;
  }

  std::string GetCppLiteral(void *data, CppProgram *prog) {
    if (sub_types_->size() == 1) {
      return (*sub_types_)[0].type->GetCppLiteral(data, prog);
    }

    std::string out;
    out += prog->GetTypeCode(shared_from_this());

    out += "(";

    for (int i = 0; i < int(sub_types_->size()); i++) {
      if (i)
        out += ", ";

      out += (*sub_types_)[i].type->GetCppLiteral(
          (char *)data + GetSubType((*sub_types_)[i].name).offset, prog);
    }

    out += ")";

    return out;
  }

  size_t GetSize() {
    size_t sum = 0;

    for (auto i : *sub_types_) {
      sum += i.type->GetSize();
    }

    return sum;
  }

  PrimitiveType GetType() { return TUPLE; }

  OffsetAndType GetSubType(std::string name) {
    size_t offset = 0;

    for (auto i : *sub_types_) {
      if (i.name == name)
        return {offset, i.type};
      else
        offset += i.type->GetSize();
    }

    return {0, nullptr};
  }

  std::vector<NamedType> *GetAllSubTypes() { return &(*sub_types_); }

  bool IsWhatev() { return hasWhatev; }

  bool IsCreatable() { return hasOnlyCreatable; }

  Type ActuallyIs(Type target) {
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
  bool MatchesSameTypeType(Type other) {
    auto o = (TupleType *)(&(*other));

    if (!isAnonymous && !o->isAnonymous)
      return false;

    if (sub_types_->size() != o->sub_types_->size())
      return false;

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
  PtrType(Type in) { type = in; }

  std::string GetString() { return "-> " + type->GetString() + " (pointer)"; }

  std::string GetCompactString() {
    return "Pp_" + type->GetCompactString() + "_pP";
  }

  std::string GetCppLiteral(void *data, CppProgram *prog) {
    std::string name = std::to_string((long long)data);
    prog->DeclareGlobal(name, type, type->GetCppLiteral(*(void **)data, prog));
    std::string out = "&" + prog->GetGlobalNames()->GetCpp(name);
    return out;
  }

  size_t GetSize() { return sizeof(void *); }

  PrimitiveType GetType() { return PTR; }

  Type GetSubType() { return type; }

  Type ActuallyIs(Type target) {
    return GetSubType()->ActuallyIs(target->GetSubType())->GetPtr();
  }

protected:
  Type type;

  bool MatchesSameTypeType(Type other) {
    return ((PtrType *)(&(*other)))->type->Matches(type);
  }
};

class MetaType : public TypeBase {
public:
  MetaType(Type in) { type = in; }

  std::string GetString() { return "{" + type->GetString() + "} (meta type)"; }

  std::string GetCompactString() {
    return "Mm_" + type->GetCompactString() + "_mM";
  }

  std::string GetCppLiteral(void *data, CppProgram *prog) {
    return "/* can't add meta type to C++ code */";
  }

  size_t GetSize() { return 0; }

  bool IsCreatable() { return false; }

  PrimitiveType GetType() { return METATYPE; }

  Type GetSubType() { return type; }

  bool IsWhatev() { return type->IsWhatev(); }

  Type ActuallyIs(Type target) {
    return GetSubType()->ActuallyIs(target->GetSubType())->GetMeta();
  }

protected:
  Type type;

  bool MatchesSameTypeType(Type other) {
    return ((MetaType *)(&(*other)))->type == type;
  }
};

class WhatevType : public TypeBase {
public:
  WhatevType() {}

  std::string GetString() { return "(whatev type)"; }

  std::string GetCompactString() { return "W"; }

  bool IsCreatable() { return false; }

  std::string GetCppLiteral(void *data, CppProgram *prog) {
    throw TerebinthError(
        "GetCppLiteral called on whatev type, wich should not have happened",
        INTERNAL_ERROR);
  }

  size_t GetSize() {
    throw TerebinthError(
        "GetSize called on whatev type, wich should not have happened",
        INTERNAL_ERROR);
  }

  PrimitiveType GetType() { return WHATEV; }

  bool IsWhatev() { return true; }

  Type GetSubType() {
    throw TerebinthError(
        "GetSubType called on whatev type, wich should not have happened",
        INTERNAL_ERROR);
  }

  Type ActuallyIs(Type target) { return target; }

protected:
  Type type;

  bool MatchesSameTypeType(Type other) { return true; }
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
  if (!PtrToMe)
    PtrToMe = Type(new PtrType(shared_from_this()));

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
  if (other == shared_from_this())
    return true;

  if (GetType() == WHATEV || other->GetType() == WHATEV)
    return true;

  if (GetType() == TUPLE) {
    auto sub_types_ = GetAllSubTypes();

    if (sub_types_->size() == 1) {
      if ((*sub_types_)[0].type->Matches(other))
        return true;
    }
  }

  if (other->GetType() == TUPLE) {
    auto sub_types_ = other->GetAllSubTypes();

    if (sub_types_->size() == 1) {
      if ((*sub_types_)[0].type->Matches(shared_from_this()))
        return true;
    }
  }

  if (other->GetType() != GetType())
    return false;

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

Type MakeTuple(const std::vector<NamedType> &in, bool isAnonymous) {
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

    if (valid)
      return str;
  }

  error_.Log("you've gotta be fuckin kidding", SOURCE_ERROR);
  return "";
}
