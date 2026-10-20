#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "execution/filter_operator.hpp"
#include "execution/table_scan_operator.hpp"

using namespace pegasus;

namespace {

Chunk MakePriceChunk() {
  Chunk chunk({"price"}, {TypeId::Double});
  for (int i = 0; i < 10; ++i) {
    chunk.AppendRow({Value{static_cast<double>(i * 10)}});
  }
  return chunk;
}
} // namespace

TEST_CASE(
    "FilterOperator selects exactly the matching rows for a known predicate",
    "[filter]") {
  Chunk chunk = MakePriceChunk();
  auto scan = std::make_unique<TableScanOperator>(chunk, /*batch_size=*/1024);
  FilterOperator filter(std::move(scan), /*column=*/0,
                        predicates::IsGreaterThan(50.0));

  filter.Open();
  VectorizedBatch batch;
  REQUIRE(filter.Next(batch));
  REQUIRE(batch.selection == std::vector<size_t>{6, 7, 8, 9});
  REQUIRE_FALSE(filter.Next(batch));
  filter.Close();
}

TEST_CASE("FilterOperator returns no batches when nothing matches",
          "[filter]") {
  Chunk chunk = MakePriceChunk();
  auto scan = std::make_unique<TableScanOperator>(chunk);
  FilterOperator filter(std::move(scan), 0, predicates::IsGreaterThan(1000.0));

  filter.Open();
  VectorizedBatch batch;
  REQUIRE_FALSE(filter.Next(batch));
  filter.Close();
}

TEST_CASE("FilterOperator LessThan and EqualsInt predicates", "[filter]") {
  Chunk chunk = MakePriceChunk();

  SECTION("LessThan") {
    auto scan = std::make_unique<TableScanOperator>(chunk);
    FilterOperator filter(std::move(scan), 0, predicates::IsLessThan(25.0));
    filter.Open();
    VectorizedBatch batch;
    REQUIRE(filter.Next(batch));
    REQUIRE(batch.selection == std::vector<size_t>{0, 1, 2});
    filter.Close();
  }

  SECTION("EqualsInt on an integer column") {
    Chunk int_chunk({"id"}, {TypeId::Integer});
    for (int64_t i = 0; i < 5; ++i)
      int_chunk.AppendRow({Value{i}});
    auto scan = std::make_unique<TableScanOperator>(int_chunk);
    FilterOperator filter(std::move(scan), 0, predicates::IsEqualInt(3));
    filter.Open();
    VectorizedBatch batch;
    REQUIRE(filter.Next(batch));
    REQUIRE(batch.selection == std::vector<size_t>{3});
    filter.Close();
  }
}

TEST_CASE("FilterOperator skips NULLs safely", "[filter]") {
  Chunk chunk({"price"}, {TypeId::Double});
  chunk.AppendRow({Value{NullType{}}});
  chunk.AppendRow({Value{100.0}});

  auto scan = std::make_unique<TableScanOperator>(chunk);
  FilterOperator filter(std::move(scan), 0, predicates::IsGreaterThan(0.0));
  filter.Open();
  VectorizedBatch batch;
  REQUIRE(filter.Next(batch));
  REQUIRE(batch.selection == std::vector<size_t>{1});
  filter.Close();
}
