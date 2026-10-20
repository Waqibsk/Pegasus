#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include "../catalog/catalog.hpp"
#include "../parser/ast.hpp"
#include "logical_plan.hpp"

namespace pegasus {

struct BindError : std::runtime_error {
  explicit BindError(const std::string &msg) : std::runtime_error(msg) {}
};

class Binder {
public:
  explicit Binder(const Catalog &catalog) : catalog_(catalog) {}

  std::unique_ptr<LogicalOperator> Bind(const SelectStmt &stmt) const;

private:
  const Catalog &catalog_;
};

} // namespace pegasus
