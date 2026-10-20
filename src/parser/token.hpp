#pragma once

#include <string>

namespace pegasus {

enum class TokenType {
  Identifier, // SELECT, FROM, WHERE, SUM, COUNT, column/table names

  IntegerLiteral,
  DoubleLiteral,
  StringLiteral,
  Comma,
  LParen,
  RParen,
  Star,
  Eq,  // =
  Neq, // !=  or  <>
  Lt,  // <
  Gt,  // >
  Lte, // <=
  Gte, // >=
  Semicolon,
  EndOfInput,
};

struct Token {
  TokenType type;
  std::string text;
};

inline const char *ToString(TokenType t) {
  switch (t) {
  case TokenType::Identifier:
    return "Identifier";
  case TokenType::IntegerLiteral:
    return "IntegerLiteral";
  case TokenType::DoubleLiteral:
    return "DoubleLiteral";
  case TokenType::StringLiteral:
    return "StringLiteral";
  case TokenType::Comma:
    return "Comma";
  case TokenType::LParen:
    return "LParen";
  case TokenType::RParen:
    return "RParen";
  case TokenType::Star:
    return "Star";
  case TokenType::Eq:
    return "Eq";
  case TokenType::Neq:
    return "Neq";
  case TokenType::Lt:
    return "Lt";
  case TokenType::Gt:
    return "Gt";
  case TokenType::Lte:
    return "Lte";
  case TokenType::Gte:
    return "Gte";
  case TokenType::Semicolon:
    return "Semicolon";
  case TokenType::EndOfInput:
    return "EndOfInput";
  }
  return "Unknown";
}

} // namespace pegasus
