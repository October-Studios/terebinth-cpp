#pragma once

#include <cstring>
#include <memory>
#include <string>
#include <vector>

class TypeBase;

typedef std::shared_ptr<TypeBase> Type;

const extern Type Unknown;
const extern Type Whatev;
const extern Type Void;
const extern Type Bool;
const extern Type Byte;
const extern Type Int;
const extern Type Double;
extern Type String;

class CppProgram;
class ActionData;

struct NamedType {
  std::string name;
  Type type;
};

struct OffsetAndType {
  size_t offset;
  Type type;
};

class TypeBase : public std::enable_shared_from_this<TypeBase> {
 public:
  virtual ~TypeBase() = default;

  enum PrimitiveType {
    UNKNOWN,
    VOID,
    BYTE,
    DOUBLE,
    INT,
    PTR,
    BOOL,
    TUPLE,
    WHATEV,
    METATYPE
  };

  static Type MakeNewVoid();

  static Type MakeNewPrimitive(PrimitiveType type_in);

  static Type MakeNewWhatev();

  Type GetMeta();

  Type GetPtr();

  virtual Type ActuallyIs(Type target);

  static std::string GetString(PrimitiveType in);

  virtual std::string GetString() = 0;

  std::string GetName() {
    return name_hint_.empty() ? GetCompactString() : name_hint_;
  }

  virtual std::string GetCompactString() = 0;

  virtual std::string GetCppLiteral(void *data, CppProgram *prog) = 0;

  virtual bool IsCreatable() { return true; }

  virtual bool IsVoid() { return false; }

  virtual bool IsWhatev() { return false; }

  virtual size_t GetSize() = 0;

  virtual PrimitiveType GetType() = 0;

  bool Matches(Type other);

  virtual Type GetSubType() { return Void; }

  virtual OffsetAndType GetSubType(std::string name) { return {0, nullptr}; }

  virtual std::vector<NamedType> *GetAllSubTypes();

  std::string name_hint_ = "";

 protected:
  virtual bool MatchesSameTypeType(Type other) = 0;

  Type PtrToMe = nullptr;
};

Type MakeTuple(const std::vector<NamedType> &in, bool is_anonymous);

class TupleTypeMaker {
 public:
  TupleTypeMaker();

  void Add(std::string name, Type type);

  void Add(Type type);

  Type Get(bool is_anonymous);

 private:
  std::string GetUniqueName();

  std::unique_ptr<std::vector<NamedType>> sub_types_;
};
