#pragma once

#include "column.hpp"
#include "value.hpp"
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace pegasus {
class Chunk {
private:
  std::vector<std::string> names_;
  std::vector<Column> columns_;

public:
  Chunk(std::vector<std::string> column_names, std::vector<TypeId> column_types)
      : names_(std::move(column_names)) {
    if (names_.size() != column_types.size()) {
      throw std::invalid_argument(
          "Chunk: column_names and column_types size mismatch ");
    }
    columns_.reserve(column_types.size());

    for (TypeId t : column_types) {
      columns_.push_back(MakeColumn(t));
    }
  }

  size_t NumColumns() const { return columns_.size(); }
  size_t NumRows() const {
    return columns_.empty() ? 0 : ColumnSize(columns_[0]);
  }
  const std::string &ColumnName(size_t col) const { return names_.at(col); }
  size_t ColumnIndex(const std::string &name) const {
    for (size_t i = 0; i < names_.size(); i++) {
      if (names_[i] == name) {
        return i;
      }
    }
    throw std::out_of_range("Chunk: no column named " + name);
  }
  const Column &GetColumn(size_t col) const { return columns_.at(col); }
  Column &GetColumn(size_t col) { return columns_.at(col); }
  Value GetValue(size_t row, size_t col) const {
    return ColumnGetValue(columns_.at(col), row);
  }

  void AppendRow(const std::vector<Value> values) {
    if (values.size() != columns_.size()) {
      throw std::invalid_argument("Chunk::AppendRow wrong number of values");
    }
    for (size_t i = 0; i < values.size(); i++) {
      ColumnAppend(columns_[i], values[i]);
    }
  }
};
} // namespace pegasus
