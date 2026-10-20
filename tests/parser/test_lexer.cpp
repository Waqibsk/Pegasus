#include <catch2/catch_test_macros.hpp>

#include "parser/lexer.hpp"

using namespace pegasus;

TEST_CASE("Lexer tokenizes a simple SELECT statement", "[lexer]") {
  auto tokens = Lexer("SELECT colA FROM table WHERE colB = 10").Tokenize();

  std::vector<TokenType> types;
  for (const auto &t : tokens)
    types.push_back(t.type);

  std::vector<TokenType> expected = {
      TokenType::Identifier, // SELECT
      TokenType::Identifier, // colA
      TokenType::Identifier, // FROM
      TokenType::Identifier, // table
      TokenType::Identifier, // WHERE
      TokenType::Identifier, // colB
      TokenType::Eq,
      TokenType::IntegerLiteral, // 10
      TokenType::EndOfInput,
  };
  REQUIRE(types == expected);
  REQUIRE(tokens[7].text == "10");
}

TEST_CASE("Lexer handles all comparison operators", "[lexer]") {
  auto tokens = Lexer("= != <> < > <= >=").Tokenize();
  std::vector<TokenType> types;
  for (const auto &t : tokens)
    types.push_back(t.type);
  REQUIRE(types ==
          std::vector<TokenType>{TokenType::Eq, TokenType::Neq, TokenType::Neq,
                                 TokenType::Lt, TokenType::Gt, TokenType::Lte,
                                 TokenType::Gte, TokenType::EndOfInput});
}

TEST_CASE("Lexer handles string and double literals", "[lexer]") {
  auto tokens = Lexer("'hello world' 3.14").Tokenize();
  REQUIRE(tokens[0].type == TokenType::StringLiteral);
  REQUIRE(tokens[0].text == "hello world");
  REQUIRE(tokens[1].type == TokenType::DoubleLiteral);
  REQUIRE(tokens[1].text == "3.14");
}

TEST_CASE("Lexer handles punctuation for function calls and lists", "[lexer]") {
  auto tokens = Lexer("SUM(price), id").Tokenize();
  std::vector<TokenType> types;
  for (const auto &t : tokens)
    types.push_back(t.type);
  REQUIRE(types ==
          std::vector<TokenType>{TokenType::Identifier, TokenType::LParen,
                                 TokenType::Identifier, TokenType::RParen,
                                 TokenType::Comma, TokenType::Identifier,
                                 TokenType::EndOfInput});
}

TEST_CASE("Lexer throws on an unterminated string literal", "[lexer]") {
  REQUIRE_THROWS_AS(Lexer("'oops").Tokenize(), LexError);
}

TEST_CASE("Lexer throws on an unexpected character", "[lexer]") {
  REQUIRE_THROWS_AS(Lexer("SELECT # FROM t").Tokenize(), LexError);
}
