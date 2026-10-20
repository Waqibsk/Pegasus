
#include "execution/filter_operator.hpp"
#include "execution/vectorized_batch.hpp"
#include <cstddef>
#include <memory>
#include <optional>
namespace pegasus {

namespace {
std::optional<double> TryToDouble(const Value &v) {
  if (IsNull(v))
    return std::nullopt;
  if (std::holds_alternative<int64_t>(v))
    return static_cast<double>(std::get<int64_t>(v));
  if (std::holds_alternative<double>(v))
    return std::get<double>(v);
  return std::nullopt;
}
} // namespace

namespace predicates {

Predicate FromCompareOp(CompareOp op, const Value &literal) {
  if (std::holds_alternative<std::string>(literal)) {
    std::string target = std::get<std::string>(literal);
    if (op == CompareOp::Eq) {
      return [target](const Value &v) {
        return !IsNull(v) && std::holds_alternative<std::string>(v) &&
               std::get<std::string>(v) == target;
      };
    }
    if (op == CompareOp::Neq) {
      return [target](const Value &v) {
        return !IsNull(v) && std::holds_alternative<std::string>(v) &&
               std::get<std::string>(v) != target;
      };
    }
    throw std::runtime_error("FromCompareOp: ordering comparisons (<, >, <=, "
                             ">=) are not supported on strings");
  }

  double threshold = std::holds_alternative<int64_t>(literal)
                         ? static_cast<double>(std::get<int64_t>(literal))
                         : std::get<double>(literal);

  switch (op) {
  case CompareOp::Eq:
    return [threshold](const Value &v) {
      auto x = TryToDouble(v);
      return x && *x == threshold;
    };
  case CompareOp::Neq:
    return [threshold](const Value &v) {
      auto x = TryToDouble(v);
      return x && *x != threshold;
    };
  case CompareOp::Lt:
    return [threshold](const Value &v) {
      auto x = TryToDouble(v);
      return x && *x < threshold;
    };
  case CompareOp::Gt:
    return [threshold](const Value &v) {
      auto x = TryToDouble(v);
      return x && *x > threshold;
    };
  case CompareOp::Lte:
    return [threshold](const Value &v) {
      auto x = TryToDouble(v);
      return x && *x <= threshold;
    };
  case CompareOp::Gte:
    return [threshold](const Value &v) {
      auto x = TryToDouble(v);
      return x && *x >= threshold;
    };
  }
  throw std::runtime_error("FromCompareOp: unreachable");
}
} // namespace predicates
FilterOperator::FilterOperator(std::unique_ptr<Operator> child,
                               int64_t column_idx, Predicate predicate)
    : child_(std::move(child)), column_idx_(std::move(column_idx)),
      predicate_(std::move(predicate)) {}

bool FilterOperator::Next(VectorizedBatch &out) {
  while (child_->Next(child_batch_)) {
    out.source = child_batch_.source;
    out.selection.clear();
    out.selection.reserve(child_batch_.Count());
    for (size_t pos = 0; pos < child_batch_.Count(); ++pos) {
      if (predicate_(child_batch_.GetValue(pos, column_idx_))) {
        out.selection.push_back(child_batch_.selection[pos]);
      }
    }
    if (!out.selection.empty()) {
      return true;
    }
  }

  return false;
}
void FilterOperator::Open() { child_->Open(); }
void FilterOperator::Close() { child_->Close(); }

} // namespace pegasus
