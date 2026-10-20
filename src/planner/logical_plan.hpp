#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../common/compare_op.hpp"
#include "../common/value.hpp"
#include "../parser/ast.hpp"

namespace pegasus {

enum class LogicalOpType { Scan, Filter, Project, Aggregate };

class LogicalOperator {
public:
  explicit LogicalOperator(LogicalOpType type) : type_(type) {}
  virtual ~LogicalOperator() = default;
  LogicalOpType Type() const { return type_; }

private:
  LogicalOpType type_;
};

class LogicalScan : public LogicalOperator {
public:
  explicit LogicalScan(std::string table_name)
      : LogicalOperator(LogicalOpType::Scan),
        table_name(std::move(table_name)) {}
  std::string table_name;
};

class LogicalFilter : public LogicalOperator {
public:
  LogicalFilter(std::unique_ptr<LogicalOperator> child, size_t column_idx,
                CompareOp op, Value literal)
      : LogicalOperator(LogicalOpType::Filter), child(std::move(child)),
        column_idx(column_idx), op(op), literal(std::move(literal)) {}
  std::unique_ptr<LogicalOperator> child;
  size_t column_idx;
  CompareOp op;
  Value literal;
};

class LogicalProject : public LogicalOperator {
public:
  LogicalProject(std::unique_ptr<LogicalOperator> child,
                 std::vector<size_t> column_indices,
                 std::vector<std::string> output_names)
      : LogicalOperator(LogicalOpType::Project), child(std::move(child)),
        column_indices(std::move(column_indices)),
        output_names(std::move(output_names)) {}
  std::unique_ptr<LogicalOperator> child;
  std::vector<size_t> column_indices;
  std::vector<std::string> output_names;
};

class LogicalAggregate : public LogicalOperator {
public:
  LogicalAggregate(std::unique_ptr<LogicalOperator> child, size_t column_idx,
                   AggFunc func, std::string output_name)
      : LogicalOperator(LogicalOpType::Aggregate), child(std::move(child)),
        column_idx(column_idx), func(func),
        output_name(std::move(output_name)) {}
  std::unique_ptr<LogicalOperator> child;
  size_t column_idx;
  AggFunc func;
  std::string output_name;
};

struct OutputColumn {
  size_t index;
  std::string name;
};

inline std::vector<OutputColumn> GetOutputColumns(const LogicalOperator &root) {
  if (root.Type() == LogicalOpType::Project) {
    const auto &p = static_cast<const LogicalProject &>(root);
    std::vector<OutputColumn> out;
    for (size_t i = 0; i < p.column_indices.size(); ++i) {
      out.push_back({p.column_indices[i], p.output_names[i]});
    }
    return out;
  }
  if (root.Type() == LogicalOpType::Aggregate) {
    const auto &a = static_cast<const LogicalAggregate &>(root);
    return {OutputColumn{0, a.output_name}};
  }
  throw std::runtime_error(
      "GetOutputColumns: plan root must be Project or Aggregate (Binder never "
      "produces a bare Scan/Filter as the top of a plan)");
}

} // namespace pegasus
