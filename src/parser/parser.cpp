#include "parser.hpp"
#include <algorithm>
#include <cctype>

namespace pegasus {

namespace {
std::string ToUpper(const std::string &s) {
  std::string out = s;
  std::transform(out.begin(), out.end(), out.begin(),
                 [](unsigned char c) { return std::toupper(c); });
  return out;
}
} // namespace

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

const Token &Parser::Peek() const { return tokens_[pos_]; }

const Token &Parser::Advance() {
  const Token &t = tokens_[pos_];
  if (pos_ + 1 < tokens_.size())
    ++pos_;
  return t;
}

bool Parser::Check(TokenType type) const { return Peek().type == type; }

const Token &Parser::Expect(TokenType type, const std::string &what) {
  if (!Check(type)) {
    throw ParseError("expected " + what + " but found '" + Peek().text + "' (" +
                     ToString(Peek().type) + ")");
  }
  return Advance();
}

bool Parser::CheckKeyword(const std::string &keyword) const {
  return Peek().type == TokenType::Identifier &&
         ToUpper(Peek().text) == ToUpper(keyword);
}

void Parser::ExpectKeyword(const std::string &keyword) {
  if (!CheckKeyword(keyword)) {
    throw ParseError("expected keyword " + keyword + " but found '" +
                     Peek().text + "'");
  }
  Advance();
}

Value Parser::ParseLiteral() {
  if (Check(TokenType::IntegerLiteral)) {
    return Value{static_cast<int64_t>(std::stoll(Advance().text))};
  }
  if (Check(TokenType::DoubleLiteral)) {
    return Value{std::stod(Advance().text)};
  }
  if (Check(TokenType::StringLiteral)) {
    return Value{Advance().text};
  }
  throw ParseError("expected a literal value but found '" + Peek().text + "'");
}

CompareOp Parser::ParseCompareOp() {
  switch (Peek().type) {
  case TokenType::Eq:
    Advance();
    return CompareOp::Eq;
  case TokenType::Neq:
    Advance();
    return CompareOp::Neq;
  case TokenType::Lt:
    Advance();
    return CompareOp::Lt;
  case TokenType::Gt:
    Advance();
    return CompareOp::Gt;
  case TokenType::Lte:
    Advance();
    return CompareOp::Lte;
  case TokenType::Gte:
    Advance();
    return CompareOp::Gte;
  default:
    throw ParseError("expected a comparison operator but found '" +
                     Peek().text + "'");
  }
}

SelectItem Parser::ParseSelectItem() {
  if (CheckKeyword("SUM") || CheckKeyword("COUNT")) {
    AggFunc func = CheckKeyword("SUM") ? AggFunc::Sum : AggFunc::Count;
    Advance(); // consume SUM/COUNT
    Expect(TokenType::LParen, "'(' after aggregate function name");
    const Token &col = Expect(TokenType::Identifier,
                              "a column name inside aggregate function");
    Expect(TokenType::RParen, "')' after aggregate function argument");
    return AggregateExpr{func, col.text};
  }
  const Token &col = Expect(TokenType::Identifier, "a column name");
  return ColumnRefExpr{col.text};
}

std::vector<SelectItem> Parser::ParseSelectList() {
  std::vector<SelectItem> items;
  items.push_back(ParseSelectItem());
  while (Check(TokenType::Comma)) {
    Advance();
    items.push_back(ParseSelectItem());
  }
  return items;
}

WhereClause Parser::ParseWhereClause() {
  const Token &col = Expect(TokenType::Identifier, "a column name after WHERE");
  CompareOp op = ParseCompareOp();
  Value literal = ParseLiteral();
  return WhereClause{col.text, op, literal};
}

SelectStmt Parser::ParseSelect() {
  ExpectKeyword("SELECT");

  SelectStmt stmt;
  if (Check(TokenType::Star)) {
    Advance();
    stmt.select_star = true;
  } else {
    stmt.items = ParseSelectList();
  }

  ExpectKeyword("FROM");
  stmt.table_name =
      Expect(TokenType::Identifier, "a table name after FROM").text;

  if (CheckKeyword("WHERE")) {
    Advance();
    stmt.where = ParseWhereClause();
  }

  if (Check(TokenType::Semicolon))
    Advance();
  Expect(TokenType::EndOfInput, "end of input (unexpected trailing tokens)");

  return stmt;
}

} // namespace pegasus
