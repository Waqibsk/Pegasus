
#include "execution/filter_operator.hpp"
#include "execution/vectorized_batch.hpp"
#include <cstddef>
#include <memory>
namespace pegasus {
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
