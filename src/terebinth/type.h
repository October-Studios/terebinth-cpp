#ifndef TEREBINTH_TYPE_H_
#define TEREBINTH_TYPE_H_

#include <cstring>
#include <memory>
#include <string>
#include <vector>


class TypeBase;

typedef std::shared_ptr<TypeBase> Type;

const extern Type Unknown;
const extern Type Void;
const extern Type Bool;

#endif // TEREBINTH_TYPE_H_