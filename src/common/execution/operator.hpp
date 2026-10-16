#pragma once

#include "common/execution/vectorized_batch.hpp"
namespace pegasus {

class Operator {

public:
  virtual ~Operator() = default;
  virtual void Open() = 0;
  virtual bool Next(VectorizedBatch &out) = 0;
  virtual void Close() = 0;
};
} // namespace pegasus
