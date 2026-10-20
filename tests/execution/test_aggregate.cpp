#include <catch2/catch_test_macros.hpp>

#include "execution/aggregate_operator.hpp"
#include "execution/filter_operator.hpp"
#include "execution/table_scan_operator.hpp"

using namespace pegasus;

namespace {
// rows: price = 10, 20, 30, 40, 50. SUM=150, COUNT=5.
Chunk MakeFiveRowChunk() {
  Chunk chunk({"price"}, {TypeId::Double});
  for (int i = 1; i <= 5; ++i)
    chunk.AppendRow({Value{static_cast<double>(i * 10)}});
  return chunk;
}
} // namespace

TEST_CASE("AggregateOperator computes SUM correctly", "[aggregate]") {
  Chunk chunk = MakeFiveRowChunk();
  auto scan = std::make_unique<TableScanOperator>(chunk);
  AggregateOperator sum_agg(std::move(scan), 0, AggrType::SUM, "sum_price");

  sum_agg.Open(); // Open() eagerly computes the aggregate
  VectorizedBatch batch;
  REQUIRE(sum_agg.Next(batch));
  REQUIRE(batch.Count() == 1);
  REQUIRE(std::get<double>(batch.GetValue(0, 0)) == 150.0);
  REQUIRE_FALSE(sum_agg.Next(batch));
  sum_agg.Close();
}

TEST_CASE("AggregateOperator computes COUNT correctly", "[aggregate]") {
  Chunk chunk = MakeFiveRowChunk();
  auto scan = std::make_unique<TableScanOperator>(chunk);
  AggregateOperator count_agg(std::move(scan), 0, AggrType::COUNT,
                              "count_price");

  count_agg.Open();
  VectorizedBatch batch;
  REQUIRE(count_agg.Next(batch));
  REQUIRE(std::get<int64_t>(batch.GetValue(0, 0)) == 5);
  count_agg.Close();
}

TEST_CASE("AggregateOperator COUNT skips NULLs", "[aggregate]") {
  Chunk chunk({"price"}, {TypeId::Double});
  chunk.AppendRow({Value{10.0}});
  chunk.AppendRow({Value{NullType{}}});
  chunk.AppendRow({Value{20.0}});

  auto scan = std::make_unique<TableScanOperator>(chunk);
  AggregateOperator count_agg(std::move(scan), 0, AggrType::COUNT,
                              "count_price");
  count_agg.Open();
  VectorizedBatch batch;
  REQUIRE(count_agg.Next(batch));
  REQUIRE(std::get<int64_t>(batch.GetValue(0, 0)) == 2);
  count_agg.Close();
}

TEST_CASE(
    "Full pipeline: scan -> filter -> aggregate matches hand-computed SUM",
    "[aggregate][integration]") {
  // 10 rows: price = 10, 20, ..., 100. WHERE price > 50 keeps 60,70,80,90,100
  // -> SUM = 400.
  Chunk chunk({"price"}, {TypeId::Double});
  for (int i = 1; i <= 10; ++i)
    chunk.AppendRow({Value{static_cast<double>(i * 10)}});

  auto scan = std::make_unique<TableScanOperator>(chunk);
  auto filter = std::make_unique<FilterOperator>(
      std::move(scan), 0, predicates::IsGreaterThan(50.0));
  AggregateOperator sum_agg(std::move(filter), 0, AggrType::SUM, "sum_price");

  sum_agg.Open();
  VectorizedBatch batch;
  REQUIRE(sum_agg.Next(batch));
  REQUIRE(std::get<double>(batch.GetValue(0, 0)) == 400.0);
  sum_agg.Close();
}
