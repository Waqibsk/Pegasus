#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "../common/chunk.hpp"

namespace pegasus {

struct ColumnDef {
  std::string name;
  TypeId type;
};

class Schema {
public:
  explicit Schema(const Chunk &chunk) {
    for (size_t i = 0; i < chunk.NumColumns(); ++i) {
      columns_.push_back({chunk.ColumnName(i), ColumnType(chunk.GetColumn(i))});
    }
  }

  size_t NumColumns() const { return columns_.size(); }
  const std::vector<ColumnDef> &Columns() const { return columns_; }

  size_t IndexOf(const std::string &name) const {
    for (size_t i = 0; i < columns_.size(); ++i) {
      if (columns_[i].name == name)
        return i;
    }
    throw std::out_of_range("Schema: no column named '" + name + "'");
  }

  bool HasColumn(const std::string &name) const {
    for (const auto &c : columns_) {
      if (c.name == name)
        return true;
    }
    return false;
  }

private:
  std::vector<ColumnDef> columns_;
};

} // namespace pegasus
