export module type;

import <cstring>;
import <memory>;
import <string>;
import <vector>;

export
class TypeBase;

export
typedef std::shared_ptr<TypeBase> Type;

export
const extern Type Unknown;
export
const extern Type Whatev;
export
const Type Void;
export
const extern Type Bool;
export
const extern Type Byte;
export
const extern Type Int;
export
const extern Type Double;
export
extern Type String;

export
class CppProgram;
export
class ActionData;

export
struct NamedType {
  std::string name;
  Type type;
};

export
struct OffsetAndType {
  size_t offset;
  Type type;
};

export
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

export
Type MakeTuple(const std::vector<NamedType> &in, bool is_anonymous);

export
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
