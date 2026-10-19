#include "project_operator.hpp"
#include "operator.hpp"
#include <memory>
#include <utility>
namespace pegasus {
ProjectOperator::ProjectOperator(std::unique_ptr<Operator> child,
                                 std::vector<size_t> column_indices,
                                 std::vector<std::string> output_names)
    : child_(std::move(child)), column_indices_(std::move(column_indices)),
      output_names_(std::move(output_names)) {}
void ProjectOperator::Open() { child_->Open(); }
bool ProjectOperator::Next(VectorizedBatch &out) { return child_->Next(out); }
void ProjectOperator::Close() { child_->Close(); }

} // namespace pegasus
