#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "ast.hpp"
#include "token.hpp"

namespace pegasus {

struct ParseError : std::runtime_error {
  explicit ParseError(const std::string &msg) : std::runtime_error(msg) {}
};

class Parser {
public:
  explicit Parser(std::vector<Token> tokens);

  SelectStmt ParseSelect();

private:
  std::vector<Token> tokens_;
  size_t pos_ = 0;

  const Token &Peek() const;
  const Token &Advance();
  bool Check(TokenType type) const;
  const Token &Expect(TokenType type, const std::string &what);

  bool CheckKeyword(const std::string &keyword) const;
  void ExpectKeyword(const std::string &keyword);

  std::vector<SelectItem> ParseSelectList();
  SelectItem ParseSelectItem();
  WhereClause ParseWhereClause();
  Value ParseLiteral();
  CompareOp ParseCompareOp();
};

} // namespace pegasus
