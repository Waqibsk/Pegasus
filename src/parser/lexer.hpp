#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "token.hpp"

namespace pegasus {

struct LexError : std::runtime_error {
  explicit LexError(const std::string &msg) : std::runtime_error(msg) {}
};

class Lexer {
public:
  explicit Lexer(std::string input);

  std::vector<Token> Tokenize();

private:
  std::string input_;
  size_t pos_ = 0;

  char Peek() const;
  char Advance();
  bool AtEnd() const;

  Token LexIdentifier();
  Token LexNumber();
  Token LexString();
};

} // namespace pegasus
