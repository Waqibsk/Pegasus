#pragma once

#include <cmath>
#include <random>

#include "../src/common/chunk.hpp"
#include "common/value.hpp"

namespace pegasus {

inline Chunk MakeMockChunk(size_t num_rows = 1000) {
  Chunk chunk({"id", "values"}, {TypeId::Integer, TypeId::Double});
  std::mt19937 rng(67);
  std::uniform_real_distribution<double> price_dist(1.0, 999.99);

  for (size_t i = 0; i < num_rows; i++) {
    double price = std::round(price_dist(rng)) * 100 / 100.0;
    chunk.AppendRow({Value{static_cast<int64_t>(i)}, Value{price}});
  }
  return chunk;
}
}; // namespace pegasus
