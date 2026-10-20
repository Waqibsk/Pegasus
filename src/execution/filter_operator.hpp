#pragma once

#include "common/compare_op.hpp"
#include "common/value.hpp"
#include "execution/vectorized_batch.hpp"
#include "operator.hpp"
#include <cstdint>
#include <functional>
#include <memory>
#include <variant>
namespace pegasus {

using Predicate = std::function<bool(const Value &)>;
namespace predicates {

inline Predicate IsGreaterThan(double Threshold) {
  return [Threshold](const Value &v) {
    if (IsNull(v))
      return false;
    if (std::holds_alternative<int64_t>(v)) {
      return std::get<int64_t>(v) > Threshold;
    }
    if (std::holds_alternative<double>(v)) {
      return std::get<double>(v) > Threshold;
    }
    return false;
  };
}
inline Predicate IsLessThan(double Threshold) {

  return [Threshold](const Value &v) {
    if (IsNull(v))
      return false;
    if (std::holds_alternative<int64_t>(v)) {
      return std::get<int64_t>(v) < Threshold;
    }
    if (std::holds_alternative<double>(v)) {
      return std::get<double>(v) < Threshold;
    }

    return false;
  };
}
inline Predicate IsEqualInt(int64_t value) {
  return [value](const Value &v) {
    return !IsNull(v) && std::holds_alternative<int64_t>(v) &&
           std::get<int64_t>(v) == value;
  };
}

Predicate FromCompareOp(CompareOp op, const Value &literal);

} // namespace predicates

// Implements WHERE. Pulls batches from `child`, evaluates `predicate` on
// `column_idx` for every row in the batch's *current* selection vector, and
// replaces it with a smaller one containing only the rows that match.
// Rows that don't match are never moved or copied

class FilterOperator : public Operator {

public:
  FilterOperator(std::unique_ptr<Operator> child, int64_t column_idx,
                 Predicate predicate);
  void Close() override;
  void Open() override;
  bool Next(VectorizedBatch &out) override;

private:
  std::unique_ptr<Operator> child_;
  int64_t column_idx_;
  Predicate predicate_;
  VectorizedBatch child_batch_;
};

} // namespace pegasus
