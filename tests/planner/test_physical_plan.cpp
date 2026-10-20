#include <catch2/catch_test_macros.hpp>

#include "catalog/catalog.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"
#include "planner/binder.hpp"
#include "planner/physical_planner.hpp"

using namespace pegasus;

namespace {

std::vector<Value> RunSql(const std::string &sql, const Catalog &catalog,
                          size_t *out_rows = nullptr) {
  SelectStmt ast = Parser(Lexer(sql).Tokenize()).ParseSelect();
  Binder binder(catalog);
  auto logical = binder.Bind(ast);
  PhysicalPlanner planner(catalog);
  auto plan = planner.Plan(*logical);

  std::vector<OutputColumn> output_cols = GetOutputColumns(*logical);

  std::vector<Value> results;
  plan->Open();
  VectorizedBatch batch;
  size_t rows = 0;
  while (plan->Next(batch)) {
    for (size_t i = 0; i < batch.Count(); ++i) {
      for (const auto &oc : output_cols)
        results.push_back(batch.GetValue(i, oc.index));
    }
    rows += batch.Count();
  }
  plan->Close();
  if (out_rows)
    *out_rows = rows;
  return results;
}

//  id = 0..9, price = 10,20,...,100
Chunk MakeTenRowChunk() {
  Chunk chunk({"id", "price"}, {TypeId::Integer, TypeId::Double});
  for (int64_t i = 0; i < 10; ++i) {
    chunk.AppendRow({Value{i}, Value{static_cast<double>((i + 1) * 10)}});
  }
  return chunk;
}

} // namespace

TEST_CASE("End-to-end: filtered projection matches hand-computed rows",
          "[integration]") {
  Chunk chunk = MakeTenRowChunk();
  Catalog catalog;
  catalog.RegisterTable("items", chunk);

  size_t rows = 0;
  // price > 70 matches price = 80, 90, 100 -> ids 7, 8, 9
  auto results =
      RunSql("SELECT id FROM items WHERE price > 70", catalog, &rows);

  REQUIRE(rows == 3);
  REQUIRE(std::get<int64_t>(results[0]) == 7);
  REQUIRE(std::get<int64_t>(results[1]) == 8);
  REQUIRE(std::get<int64_t>(results[2]) == 9);
}

TEST_CASE("End-to-end: SUM aggregate matches hand-computed total",
          "[integration]") {
  Chunk chunk = MakeTenRowChunk();
  Catalog catalog;
  catalog.RegisterTable("items", chunk);

  // price > 70 keeps 80+90+100 = 270
  auto results =
      RunSql("SELECT SUM(price) FROM items WHERE price > 70", catalog);
  REQUIRE(results.size() == 1);
  REQUIRE(std::get<double>(results[0]) == 270.0);
}

TEST_CASE("End-to-end: COUNT aggregate matches hand-computed count",
          "[integration]") {
  Chunk chunk = MakeTenRowChunk();
  Catalog catalog;
  catalog.RegisterTable("items", chunk);

  auto results =
      RunSql("SELECT COUNT(id) FROM items WHERE price <= 30", catalog);
  REQUIRE(results.size() == 1);
  REQUIRE(std::get<int64_t>(results[0]) == 3); // price 10, 20, 30
}

TEST_CASE("End-to-end: SELECT * returns every column", "[integration]") {
  Chunk chunk = MakeTenRowChunk();
  Catalog catalog;
  catalog.RegisterTable("items", chunk);

  size_t rows = 0;
  auto results = RunSql("SELECT * FROM items", catalog, &rows);
  REQUIRE(rows == 10);
  REQUIRE(results.size() == 20); // 10 rows x 2 columns
}

TEST_CASE(
    "End-to-end: a query with no matching rows returns zero rows, not an error",
    "[integration]") {
  Chunk chunk = MakeTenRowChunk();
  Catalog catalog;
  catalog.RegisterTable("items", chunk);

  size_t rows = 0;
  auto results =
      RunSql("SELECT id FROM items WHERE price > 10000", catalog, &rows);
  REQUIRE(rows == 0);
  REQUIRE(results.empty());
}
