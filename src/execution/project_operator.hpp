#pragma once

#include "execution/vectorized_batch.hpp"
#include "operator.hpp"
#include <memory>
#include <stddef.h>
#include <string>
#include <vector>
namespace pegasus {

class ProjectOperator : public Operator {

public:
  ProjectOperator(std::unique_ptr<Operator> child,
                  std::vector<size_t> column_indices,
                  std::vector<std::string> output_names);

  void Open() override;
  bool Next(VectorizedBatch &out) override;
  void Close() override;
  const std::vector<size_t> &ProjectedColumns() { return column_indices_; }
  const std::vector<std::string> &OutputNames() { return output_names_; }

private:
  std::unique_ptr<Operator> child_;
  std::vector<size_t> column_indices_;
  std::vector<std::string> output_names_;
};

} // namespace pegasus
