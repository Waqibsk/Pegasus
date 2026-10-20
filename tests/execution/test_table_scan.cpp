#include <catch2/catch_test_macros.hpp>

#include "execution/table_scan_operator.hpp"

using namespace pegasus;

namespace {
Chunk MakeSmallChunk(size_t rows) {
  Chunk chunk({"id"}, {TypeId::Integer});
  for (size_t i = 0; i < rows; ++i) {
    chunk.AppendRow({Value{static_cast<int64_t>(i)}});
  }
  return chunk;
}
} // namespace

TEST_CASE("TableScanOperator yields batches of the requested size", "[scan]") {
  Chunk chunk = MakeSmallChunk(2500);
  TableScanOperator scan(chunk, /*batch_size=*/1024);
  scan.Open();

  VectorizedBatch batch;
  REQUIRE(scan.Next(batch));
  REQUIRE(batch.Count() == 1024);
  REQUIRE(batch.selection.front() == 0);
  REQUIRE(batch.selection.back() == 1023);

  REQUIRE(scan.Next(batch));
  REQUIRE(batch.Count() == 1024);
  REQUIRE(batch.selection.front() == 1024);

  REQUIRE(scan.Next(batch));
  REQUIRE(batch.Count() == 452);
  REQUIRE_FALSE(scan.Next(batch));
  scan.Close();
}

TEST_CASE("TableScanOperator on an empty chunk yields no batches", "[scan]") {
  Chunk chunk = MakeSmallChunk(0);
  TableScanOperator scan(chunk);
  scan.Open();
  VectorizedBatch batch;
  REQUIRE_FALSE(scan.Next(batch));
  scan.Close();
}

TEST_CASE("TableScanOperator is re-runnable after Open()", "[scan]") {
  Chunk chunk = MakeSmallChunk(10);
  TableScanOperator scan(chunk, /*batch_size=*/100);
  scan.Open();
  VectorizedBatch batch;
  REQUIRE(scan.Next(batch));
  REQUIRE(batch.Count() == 10);
  REQUIRE_FALSE(scan.Next(batch));

  scan.Open();
  REQUIRE(scan.Next(batch));
  REQUIRE(batch.Count() == 10);
  scan.Close();
}
