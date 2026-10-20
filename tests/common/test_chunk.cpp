#include <catch2/catch_test_macros.hpp>

#include "../data/mock_generator.hpp"
#include "common/chunk.hpp"

using namespace pegasus;

TEST_CASE("Chunk holds mixed-type rows with correct row/column counts",
          "[chunk]") {
  Chunk chunk({"id", "name", "score"},
              {TypeId::Integer, TypeId::Varchar, TypeId::Double});

  chunk.AppendRow({Value{int64_t{1}}, Value{std::string("alice")}, Value{9.5}});
  chunk.AppendRow({Value{int64_t{2}}, Value{std::string("bob")}, Value{7.25}});

  REQUIRE(chunk.NumColumns() == 3);
  REQUIRE(chunk.NumRows() == 2);

  REQUIRE(std::get<int64_t>(chunk.GetValue(0, 0)) == 1);
  REQUIRE(std::get<std::string>(chunk.GetValue(0, 1)) == "alice");
  REQUIRE(std::get<double>(chunk.GetValue(0, 2)) == 9.5);

  REQUIRE(std::get<int64_t>(chunk.GetValue(1, 0)) == 2);
  REQUIRE(std::get<std::string>(chunk.GetValue(1, 1)) == "bob");
}

TEST_CASE("Chunk resolves column names to indices", "[chunk]") {
  Chunk chunk({"id", "price"}, {TypeId::Integer, TypeId::Double});
  REQUIRE(chunk.ColumnIndex("id") == 0);
  REQUIRE(chunk.ColumnIndex("price") == 1);
  REQUIRE_THROWS_AS(chunk.ColumnIndex("nope"), std::out_of_range);
}

TEST_CASE("Chunk stores and retrieves NULL values", "[chunk]") {
  Chunk chunk({"a"}, {TypeId::Integer});
  chunk.AppendRow({Value{NullType{}}});
  REQUIRE(chunk.NumRows() == 1);
  REQUIRE(IsNull(chunk.GetValue(0, 0)));
}

TEST_CASE("AppendRow rejects the wrong number of values", "[chunk]") {
  Chunk chunk({"a", "b"}, {TypeId::Integer, TypeId::Double});
  REQUIRE_THROWS_AS(chunk.AppendRow({Value{int64_t{1}}}),
                    std::invalid_argument);
}

TEST_CASE("MakeMockChunk produces exactly the requested number of rows",
          "[chunk][mock]") {
  Chunk chunk = MakeMockChunk(10000);
  REQUIRE(chunk.NumRows() == 10000);
  REQUIRE(chunk.NumColumns() == 2);

  // IDs should be sequential starting at 0.
  REQUIRE(std::get<int64_t>(chunk.GetValue(0, 0)) == 0);
  REQUIRE(std::get<int64_t>(chunk.GetValue(9999, 0)) == 9999);

  // Prices should fall within the generator's configured range.
  for (size_t row : {size_t{0}, size_t{4999}, size_t{9999}}) {
    double price = std::get<double>(chunk.GetValue(row, 1));
    REQUIRE(price >= 1.0);
    REQUIRE(price <= 999.99);
  }
}
