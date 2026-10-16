#include <iostream>
#include <variant>

#include "../data/mock_generator.hpp"
#include "common/chunk.hpp"

namespace {

void PrintValue(const pegasus::Value &v) {
  std::visit(
      [](const auto &x) {
        using T = std::decay_t<decltype(x)>;
        if constexpr (std::is_same_v<T, pegasus::NullType>) {
          std::cout << "NULL";
        } else {
          std::cout << x;
        }
      },
      v);
}

} // namespace

int main() {
  pegasus::Chunk chunk = pegasus::MakeMockChunk(10000);

  std::cout << "Built a chunk with " << chunk.NumRows() << " rows, "
            << chunk.NumColumns() << " columns.\n";
  std::cout << "First 5 rows (id, price):\n";
  for (size_t row = 0; row < 5; ++row) {
    PrintValue(chunk.GetValue(row, 0));
    std::cout << ", ";
    PrintValue(chunk.GetValue(row, 1));
    std::cout << "\n";
  }
  return 0;
}
