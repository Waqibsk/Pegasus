#pragma once

#include "common/chunk.hpp"
#include "common/execution/operator.hpp"
#include "common/execution/vectorized_batch.hpp"
#include <cstddef>
#include <stddef.h>

namespace pegasus {

class TableScanOperator : public Operator {

public:
  static constexpr size_t KDefaultBatchSize = 1024;

  explicit TableScanOperator(const Chunk &chunk,
                             size_t batch_size = KDefaultBatchSize);

  bool Next(VectorizedBatch &out) override;
  void Open() override;
  void Close() override;

private:
  const Chunk &chunk_;
  size_t batch_size_;
  size_t cursor_ = 0;
};

} // namespace pegasus
