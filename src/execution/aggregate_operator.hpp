
#include "execution/operator.hpp"
#include "execution/vectorized_batch.hpp"
#include <cstdint>
#include <memory>
#include <string>
namespace pegasus {

enum class AggrType { SUM, COUNT };

class AggregateOperator : public Operator {
public:
  AggregateOperator(std::unique_ptr<Operator> child, std::int64_t column_idx,
                    AggrType aggrtype, std::string output_name);
  void Open() override;
  void Close() override;
  bool Next(VectorizedBatch &out) override;

private:
  std::unique_ptr<Operator> child_;
  std::unique_ptr<Chunk> result_;

  std::int64_t column_idx_;
  AggrType aggrtype_;
  std::string output_name_;
  bool produced_ = false;
};
} // namespace pegasus
