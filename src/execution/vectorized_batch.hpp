#pragma once

#include <cstddef>
#include <stddef.h>
#include <vector>

#include "common/chunk.hpp"

namespace pegasus {

struct VectorizedBatch {
  const Chunk *source = nullptr;
  std::vector<size_t> selection;
  size_t Count() const { return selection.size(); }
  Value GetValue(size_t selection_pos, size_t col) {
    return source->GetValue(selection[selection_pos], col);
  }
};

}; // namespace pegasus
