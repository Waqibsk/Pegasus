#pragma once

#include "value.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace pegasus {

template <typename T>

struct TypedColumn {
  std::vector<T> data;
  std::vector<bool> valid;

  size_t Size() const { return data.size(); }

  void Append(const T &v) {
    data.push_back(v);
    valid.push_back(true);
  }

  void AppendNull() {
    data.emplace_back();
    valid.push_back(false);
  }

  Value GetValue(size_t row) const {
    if (row >= data.size())
      throw std::out_of_range("TypedColumn:GetValue row out of range");
    if (!valid[row])
      return Value(NullType());
    return Value{data[row]};
  }
};

using Column = std::variant<TypedColumn<int64_t>, TypedColumn<double>,
                            TypedColumn<std::string>>;

inline Column MakeColumn(TypeId type) {
  switch (type) {
  case TypeId::Integer:
    return Column{TypedColumn<int64_t>{}};
  case TypeId::Varchar:
    return Column{TypedColumn<std::string>{}};
  case TypeId::Double:
    return Column{TypedColumn<double>{}};
  }
}
inline size_t ColumnSize(const Column &c) {
  return std::visit([](const auto &typed) { return typed.Size(); }, c);
}
inline Value ColumnGetValue(const Column &c, size_t row) {
  return std::visit([row](const auto &typed) { return typed.GetValue(row); },
                    c);
}
} // namespace pegasus
