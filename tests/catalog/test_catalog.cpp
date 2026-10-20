#include <catch2/catch_test_macros.hpp>

#include "catalog/catalog.hpp"

using namespace pegasus;

TEST_CASE("Catalog registers a table and exposes its schema", "[catalog]") {
  Chunk chunk({"id", "price"}, {TypeId::Integer, TypeId::Double});
  chunk.AppendRow({Value{int64_t{1}}, Value{9.99}});

  Catalog catalog;
  REQUIRE_FALSE(catalog.HasTable("items"));
  catalog.RegisterTable("items", chunk);
  REQUIRE(catalog.HasTable("items"));

  const Schema &schema = catalog.GetSchema("items");
  REQUIRE(schema.NumColumns() == 2);
  REQUIRE(schema.HasColumn("id"));
  REQUIRE(schema.HasColumn("price"));
  REQUIRE_FALSE(schema.HasColumn("nonexistent"));
  REQUIRE(schema.Columns()[0].type == TypeId::Integer);
  REQUIRE(schema.Columns()[1].type == TypeId::Double);
}

TEST_CASE("Catalog::GetChunk returns the exact registered chunk", "[catalog]") {
  Chunk chunk({"id"}, {TypeId::Integer});
  chunk.AppendRow({Value{int64_t{42}}});

  Catalog catalog;
  catalog.RegisterTable("t", chunk);
  REQUIRE(&catalog.GetChunk("t") == &chunk);
}

TEST_CASE("Catalog throws on lookup of an unregistered table", "[catalog]") {
  Catalog catalog;
  REQUIRE_THROWS_AS(catalog.GetSchema("nope"), std::out_of_range);
  REQUIRE_THROWS_AS(catalog.GetChunk("nope"), std::out_of_range);
}

TEST_CASE("Schema::IndexOf throws for an unknown column", "[catalog]") {
  Chunk chunk({"a"}, {TypeId::Integer});
  chunk.AppendRow({Value{int64_t{1}}});
  Schema schema(chunk);
  REQUIRE_THROWS_AS(schema.IndexOf("b"), std::out_of_range);
}
