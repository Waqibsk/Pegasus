#include <catch2/catch_test_macros.hpp>

#include "catalog/catalog.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"
#include "planner/binder.hpp"

using namespace pegasus;

namespace {
Chunk MakeItemsChunk() {
  Chunk chunk({"id", "price"}, {TypeId::Integer, TypeId::Double});
  chunk.AppendRow({Value{int64_t{1}}, Value{9.99}});
  return chunk;
}

Catalog MakeItemsCatalog(const Chunk &storage) {
  Catalog catalog;
  catalog.RegisterTable("items", storage);
  return catalog;
}

SelectStmt Parse(const std::string &sql) {
  return Parser(Lexer(sql).Tokenize()).ParseSelect();
}
} // namespace

TEST_CASE("Binder produces LogicalProject over LogicalFilter over LogicalScan",
          "[binder]") {
  Chunk storage = MakeItemsChunk();
  Catalog catalog = MakeItemsCatalog(storage);
  Binder binder(catalog);

  auto plan = binder.Bind(Parse("SELECT id, price FROM items WHERE price > 5"));

  REQUIRE(plan->Type() == LogicalOpType::Project);
  const auto &project = static_cast<const LogicalProject &>(*plan);
  REQUIRE(project.column_indices == std::vector<size_t>{0, 1});

  REQUIRE(project.child->Type() == LogicalOpType::Filter);
  const auto &filter = static_cast<const LogicalFilter &>(*project.child);
  REQUIRE(filter.column_idx == 1);
  REQUIRE(filter.op == CompareOp::Gt);

  REQUIRE(filter.child->Type() == LogicalOpType::Scan);
  REQUIRE(static_cast<const LogicalScan &>(*filter.child).table_name ==
          "items");
}

TEST_CASE("Binder produces LogicalAggregate for an aggregate query",
          "[binder]") {
  Chunk storage = MakeItemsChunk();
  Catalog catalog = MakeItemsCatalog(storage);
  Binder binder(catalog);

  auto plan = binder.Bind(Parse("SELECT SUM(price) FROM items"));
  REQUIRE(plan->Type() == LogicalOpType::Aggregate);
  const auto &agg = static_cast<const LogicalAggregate &>(*plan);
  REQUIRE(agg.column_idx == 1);
  REQUIRE(agg.func == AggFunc::Sum);
}

TEST_CASE("Binder throws on an unknown table", "[binder]") {
  Chunk storage = MakeItemsChunk();
  Catalog catalog = MakeItemsCatalog(storage);
  Binder binder(catalog);
  REQUIRE_THROWS_AS(binder.Bind(Parse("SELECT id FROM ghost_table")),
                    BindError);
}

TEST_CASE("Binder throws on an unknown column in SELECT", "[binder]") {
  Chunk storage = MakeItemsChunk();
  Catalog catalog = MakeItemsCatalog(storage);
  Binder binder(catalog);
  REQUIRE_THROWS_AS(binder.Bind(Parse("SELECT nonexistent FROM items")),
                    BindError);
}

TEST_CASE("Binder throws on an unknown column in WHERE", "[binder]") {
  Chunk storage = MakeItemsChunk();
  Catalog catalog = MakeItemsCatalog(storage);
  Binder binder(catalog);
  REQUIRE_THROWS_AS(binder.Bind(Parse("SELECT id FROM items WHERE ghost = 1")),
                    BindError);
}

TEST_CASE("Binder throws on a WHERE type mismatch", "[binder]") {
  Chunk storage = MakeItemsChunk();
  Catalog catalog = MakeItemsCatalog(storage);
  Binder binder(catalog);
  // "id" is Integer; comparing it to a string literal should be rejected.
  REQUIRE_THROWS_AS(
      binder.Bind(Parse("SELECT id FROM items WHERE id = 'oops'")), BindError);
}

TEST_CASE("Binder rejects mixing aggregate and non-aggregate select items",
          "[binder]") {
  Chunk storage = MakeItemsChunk();
  Catalog catalog = MakeItemsCatalog(storage);
  Binder binder(catalog);
  REQUIRE_THROWS_AS(binder.Bind(Parse("SELECT id, SUM(price) FROM items")),
                    BindError);
}
