#pragma once

#include <memory>

#include "../catalog/catalog.hpp"
#include "../execution/operator.hpp"
#include "logical_plan.hpp"

namespace pegasus {

class PhysicalPlanner {
public:
  explicit PhysicalPlanner(const Catalog &catalog) : catalog_(catalog) {}

  std::unique_ptr<Operator> Plan(const LogicalOperator &logical) const;

private:
  const Catalog &catalog_;
};

} // namespace pegasus
