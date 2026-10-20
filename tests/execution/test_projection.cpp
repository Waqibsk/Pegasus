#include <catch2/catch_test_macros.hpp>

#include "execution/project_operator.hpp"
#include "execution/table_scan_operator.hpp"

using namespace pegasus;

TEST_CASE("ProjectOperator reports the requested output columns and names",
          "[project]") {
  Chunk chunk({"id", "name", "price"},
              {TypeId::Integer, TypeId::Varchar, TypeId::Double});
  chunk.AppendRow({Value{int64_t{1}}, Value{std::string("a")}, Value{9.99}});

  auto scan = std::make_unique<TableScanOperator>(chunk);
  ProjectOperator project(std::move(scan), {2, 0}, {"price", "id"});

  REQUIRE(project.ProjectedColumns() == std::vector<size_t>{2, 0});
  REQUIRE(project.OutputNames() == std::vector<std::string>{"price", "id"});
}

TEST_CASE("ProjectOperator passes rows through unchanged", "[project]") {
  Chunk chunk({"id"}, {TypeId::Integer});
  for (int64_t i = 0; i < 5; ++i)
    chunk.AppendRow({Value{i}});

  auto scan = std::make_unique<TableScanOperator>(chunk);
  ProjectOperator project(std::move(scan), {0}, {"id"});

  project.Open();
  VectorizedBatch batch;
  REQUIRE(project.Next(batch));
  REQUIRE(batch.Count() == 5);
  REQUIRE(std::get<int64_t>(batch.GetValue(0, 0)) == 0);
  REQUIRE(std::get<int64_t>(batch.GetValue(4, 0)) == 4);
  project.Close();
}

TEST_CASE("ProjectOperator rejects mismatched column/name counts",
          "[project]") {
  Chunk chunk({"id"}, {TypeId::Integer});
  auto scan = std::make_unique<TableScanOperator>(chunk);
  REQUIRE_THROWS_AS(ProjectOperator(std::move(scan), {0, 1}, {"only_one"}),
                    std::invalid_argument);
}
