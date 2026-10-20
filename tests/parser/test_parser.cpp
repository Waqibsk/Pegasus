#include <catch2/catch_test_macros.hpp>

#include "parser/lexer.hpp"
#include "parser/parser.hpp"

using namespace pegasus;

namespace {
SelectStmt Parse(const std::string &sql) {
  return Parser(Lexer(sql).Tokenize()).ParseSelect();
}
} // namespace

TEST_CASE("Parser builds the correct AST for a simple filtered select",
          "[parser]") {
  SelectStmt stmt = Parse("SELECT colA FROM table WHERE colB = 10");

  REQUIRE_FALSE(stmt.select_star);
  REQUIRE(stmt.table_name == "table");
  REQUIRE(stmt.items.size() == 1);
  REQUIRE(std::holds_alternative<ColumnRefExpr>(stmt.items[0]));
  REQUIRE(std::get<ColumnRefExpr>(stmt.items[0]).column_name == "colA");

  REQUIRE(stmt.where.has_value());
  REQUIRE(stmt.where->column_name == "colB");
  REQUIRE(stmt.where->op == CompareOp::Eq);
  REQUIRE(std::get<int64_t>(stmt.where->literal) == 10);
}

TEST_CASE("Parser handles multiple select columns", "[parser]") {
  SelectStmt stmt = Parse("SELECT id, price FROM items");
  REQUIRE(stmt.items.size() == 2);
  REQUIRE(std::get<ColumnRefExpr>(stmt.items[0]).column_name == "id");
  REQUIRE(std::get<ColumnRefExpr>(stmt.items[1]).column_name == "price");
  REQUIRE_FALSE(stmt.where.has_value());
}

TEST_CASE("Parser handles SELECT *", "[parser]") {
  SelectStmt stmt = Parse("SELECT * FROM items");
  REQUIRE(stmt.select_star);
  REQUIRE(stmt.items.empty());
}

TEST_CASE("Parser handles aggregate functions", "[parser]") {
  SelectStmt stmt = Parse("SELECT SUM(price) FROM items WHERE price > 500");
  REQUIRE(stmt.items.size() == 1);
  REQUIRE(std::holds_alternative<AggregateExpr>(stmt.items[0]));
  const auto &agg = std::get<AggregateExpr>(stmt.items[0]);
  REQUIRE(agg.func == AggFunc::Sum);
  REQUIRE(agg.column_name == "price");
  REQUIRE(stmt.where->op == CompareOp::Gt);
  REQUIRE(std::get<int64_t>(stmt.where->literal) == 500);
}

TEST_CASE("Parser handles string literals and a trailing semicolon",
          "[parser]") {
  SelectStmt stmt = Parse("SELECT name FROM users WHERE name = 'alice';");
  REQUIRE(std::get<std::string>(stmt.where->literal) == "alice");
}

TEST_CASE("Parser throws on missing FROM", "[parser]") {
  REQUIRE_THROWS_AS(Parse("SELECT id items"), ParseError);
}

TEST_CASE("Parser throws on missing comma between select items", "[parser]") {
  REQUIRE_THROWS_AS(Parse("SELECT id price FROM items"), ParseError);
}

TEST_CASE("Parser throws on unexpected trailing tokens", "[parser]") {
  REQUIRE_THROWS_AS(Parse("SELECT id FROM items WHERE id = 1 garbage"),
                    ParseError);
}

TEST_CASE("Parser throws on a malformed aggregate call", "[parser]") {
  REQUIRE_THROWS_AS(Parse("SELECT SUM price) FROM items"), ParseError);
}
