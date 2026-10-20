#include "lexer.hpp"

#include <cctype>

namespace pegasus {

Lexer::Lexer(std::string input) : input_(std::move(input)) {}

bool Lexer::AtEnd() const { return pos_ >= input_.size(); }

char Lexer::Peek() const { return AtEnd() ? '\0' : input_[pos_]; }

char Lexer::Advance() { return input_[pos_++]; }

Token Lexer::LexIdentifier() {
  size_t start = pos_;
  while (!AtEnd() &&
         (std::isalnum(static_cast<unsigned char>(Peek())) || Peek() == '_')) {
    Advance();
  }
  return Token{TokenType::Identifier, input_.substr(start, pos_ - start)};
}

Token Lexer::LexNumber() {
  size_t start = pos_;
  bool is_double = false;
  while (!AtEnd() && std::isdigit(static_cast<unsigned char>(Peek())))
    Advance();
  if (!AtEnd() && Peek() == '.') {
    is_double = true;
    Advance();
    while (!AtEnd() && std::isdigit(static_cast<unsigned char>(Peek())))
      Advance();
  }
  std::string text = input_.substr(start, pos_ - start);
  return Token{is_double ? TokenType::DoubleLiteral : TokenType::IntegerLiteral,
               text};
}

Token Lexer::LexString() {
  Advance();
  size_t start = pos_;
  while (!AtEnd() && Peek() != '\'')
    Advance();
  if (AtEnd())
    throw LexError("unterminated string literal");
  std::string text = input_.substr(start, pos_ - start);
  Advance();
  return Token{TokenType::StringLiteral, text};
}

std::vector<Token> Lexer::Tokenize() {
  std::vector<Token> tokens;

  while (!AtEnd()) {
    char c = Peek();

    if (std::isspace(static_cast<unsigned char>(c))) {
      Advance();
      continue;
    }
    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
      tokens.push_back(LexIdentifier());
      continue;
    }
    if (std::isdigit(static_cast<unsigned char>(c))) {
      tokens.push_back(LexNumber());
      continue;
    }
    if (c == '\'') {
      tokens.push_back(LexString());
      continue;
    }

    switch (c) {
    case ',':
      Advance();
      tokens.push_back({TokenType::Comma, ","});
      continue;
    case '(':
      Advance();
      tokens.push_back({TokenType::LParen, "("});
      continue;
    case ')':
      Advance();
      tokens.push_back({TokenType::RParen, ")"});
      continue;
    case '*':
      Advance();
      tokens.push_back({TokenType::Star, "*"});
      continue;
    case ';':
      Advance();
      tokens.push_back({TokenType::Semicolon, ";"});
      continue;
    case '=':
      Advance();
      tokens.push_back({TokenType::Eq, "="});
      continue;
    case '<': {
      Advance();
      if (!AtEnd() && Peek() == '=') {
        Advance();
        tokens.push_back({TokenType::Lte, "<="});
      } else if (!AtEnd() && Peek() == '>') {
        Advance();
        tokens.push_back({TokenType::Neq, "<>"});
      } else
        tokens.push_back({TokenType::Lt, "<"});
      continue;
    }
    case '>': {
      Advance();
      if (!AtEnd() && Peek() == '=') {
        Advance();
        tokens.push_back({TokenType::Gte, ">="});
      } else
        tokens.push_back({TokenType::Gt, ">"});
      continue;
    }
    case '!': {
      Advance();
      if (!AtEnd() && Peek() == '=') {
        Advance();
        tokens.push_back({TokenType::Neq, "!="});
        continue;
      }
      throw LexError("unexpected '!' (did you mean '!='?)");
    }
    default:
      throw LexError(std::string("unexpected character '") + c + "'");
    }
  }

  tokens.push_back({TokenType::EndOfInput, ""});
  return tokens;
}

} // namespace pegasus
