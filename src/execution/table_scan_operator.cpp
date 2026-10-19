
#include "table_scan_operator.hpp"
#include "vectorized_batch.hpp"
#include <algorithm>
#include <cstddef>
namespace pegasus {
TableScanOperator::TableScanOperator(const Chunk &chunk, size_t batch_size)
    : chunk_(chunk), batch_size_(batch_size) {}

bool TableScanOperator::Next(VectorizedBatch &out) {
  if (cursor_ > chunk_.NumRows()) {
    return false;
  }
  size_t end = std::min(cursor_ + batch_size_, chunk_.NumRows());
  out.source = &chunk_;
  out.selection.clear();
  out.selection.reserve(end - cursor_);
  for (size_t i = 0; i < end; i++) {
    out.selection.push_back(i);
  }
  cursor_ = end;
  return true;
}
void TableScanOperator::Open() { cursor_ = 0; }
void TableScanOperator::Close() {}

} // namespace pegasus
