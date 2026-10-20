// Benchmarks pegasus on a 5M-row dataset (overridable via argv[1]) with two
// aggregate queries. Times the full path a user would pay for:
// lex -> parse -> bind -> physical plan -> execute.
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "../data/mock_generator.hpp"
#include "catalog/catalog.hpp"
#include "parser/lexer.hpp"
#include "parser/parser.hpp"
#include "planner/binder.hpp"
#include "planner/physical_planner.hpp"

using namespace pegasus;
using Clock = std::chrono::steady_clock;

static Value RunOnce(const std::string &sql, const Catalog &catalog) {
  SelectStmt ast = Parser(Lexer(sql).Tokenize()).ParseSelect();
  Binder binder(catalog);
  auto logical = binder.Bind(ast);
  auto plan = PhysicalPlanner(catalog).Plan(*logical);
  auto cols = GetOutputColumns(*logical);

  plan->Open();
  VectorizedBatch batch;
  Value result = Value{NullType{}};
  while (plan->Next(batch))
    result = batch.GetValue(0, cols[0].index);
  plan->Close();
  return result;
}

int main(int argc, char **argv) {
  size_t rows = argc > 1 ? std::strtoull(argv[1], nullptr, 10) : 5000000;

  auto t0 = Clock::now();
  Chunk items = MakeMockChunk(rows);
  double load_s = std::chrono::duration<double>(Clock::now() - t0).count();
  Catalog catalog;
  catalog.RegisterTable("items", items);
  std::printf("loaded %zu rows in %.2fs (not part of query timings)\n", rows,
              load_s);

  const std::vector<std::string> queries = {
      "SELECT SUM(price) FROM items WHERE price > 500",
      "SELECT COUNT(id) FROM items WHERE price <= 100",
  };

  for (const auto &q : queries) {
    RunOnce(q, catalog);
    std::vector<double> ms;
    Value last;
    for (int i = 0; i < 3; ++i) {
      auto s = Clock::now();
      last = RunOnce(q, catalog);
      ms.push_back(
          std::chrono::duration<double, std::milli>(Clock::now() - s).count());
    }
    std::sort(ms.begin(), ms.end());
    if (std::holds_alternative<double>(last))
      std::printf("%-50s result=%.4f  runs(ms)=%.1f %.1f %.1f  median=%.1f\n",
                  q.c_str(), std::get<double>(last), ms[0], ms[1], ms[2],
                  ms[1]);
    else
      std::printf("%-50s result=%lld  runs(ms)=%.1f %.1f %.1f  median=%.1f\n",
                  q.c_str(), (long long)std::get<int64_t>(last), ms[0], ms[1],
                  ms[2], ms[1]);
  }
  return 0;
}
