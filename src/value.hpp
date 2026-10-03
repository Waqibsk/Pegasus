#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <variant>

namespace pegasus {

struct NullType {
  bool operator==(const NullType &) const { return true; }
};

enum class TypeId : uint8_t {
  Integer,
  Double,
  Varchar,
};

using Value = std::variant<NullType, int64_t, double, std::string>;

inline TypeId TypeIdOf(const Value &v) {
  if (std::holds_alternative<int64_t>(v)) {
    return TypeId::Integer;
  }
  if (std::holds_alternative<double>(v)) {
    return TypeId::Double;
  }
  if (std::holds_alternative<std::string>(v)) {
    return TypeId::Varchar;
  }
  throw std::runtime_error("TypeIdOf called on a NULL value");
}

inline bool IsNull(const Value &v) {
  return std::holds_alternative<NullType>(v);
}

inline const char *ToString(TypeId t) {
  switch (t) {
  case TypeId::Integer:
    return "INTEGER";
  case TypeId::Double:
    return "DOUBLE";
  case TypeId::Varchar:
    return "VARCHAR";
  }
  throw std::runtime_error("unreachable");
}
} // namespace pegasus
