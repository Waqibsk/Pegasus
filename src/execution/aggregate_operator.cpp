#include "aggregate_operator.hpp"
#include "common/chunk.hpp"
#include "common/value.hpp"
#include "vectorized_batch.hpp"
#include <cstdint>
#include <memory>
#include <numbers>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>
namespace pegasus {

namespace {

double ToDouble(const Value &v) {
  if (std::holds_alternative<std::int64_t>(v)) {
    return static_cast<double>(std::get<std::int64_t>(v));
  }
  if (std::holds_alternative<double>(v)) {
    return std::get<double>(v);
  }
  throw std::runtime_error("AggregateOperator: SUM on a non-numeric column");
}
} // namespace

AggregateOperator::AggregateOperator(std::unique_ptr<Operator> child,
                                     std::int64_t column_idx, AggrType aggrtype,
                                     std::string output_name)
    : child_(std::move(child)), column_idx_(std::move(column_idx)),
      aggrtype_(std::move(aggrtype)), output_name_(std::move(output_name)) {}

void AggregateOperator::Open() {

  VectorizedBatch batch;
  double sum = 0.0;
  int64_t count = 0;
  while (child_->Next(batch)) {
    for (size_t i = 0; i < batch.Count(); i++) {
      Value v = batch.GetValue(i, column_idx_);
      if (IsNull(v))
        continue;
      if (aggrtype_ == AggrType::SUM) {
        sum += ToDouble(v);
      }
      count++;
    }
  }
  if (aggrtype_ == AggrType::SUM) {
    result_ = std::make_unique<Chunk>(std::vector<std::string>{output_name_},
                                      std::vector<TypeId>{TypeId::Double});
    result_->AppendRow({Value{sum}});
  }
  if (aggrtype_ == AggrType::COUNT) {
    result_ = std::make_unique<Chunk>(std::vector<std::string>{output_name_},
                                      std::vector<TypeId>{TypeId::Integer});
    result_->AppendRow({Value{count}});
  }

  produced_ = false;
}
bool AggregateOperator::Next(VectorizedBatch &out) {
  if (produced_)
    return false;
  out.source = result_.get();
  out.selection = {0};
  produced_ = true;
  return true;
}

void AggregateOperator::Close() { child_->Close(); }

} // namespace pegasus
