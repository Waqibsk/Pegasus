#include "physical_planner.hpp"

#include <stdexcept>

#include "../execution/aggregate_operator.hpp"
#include "../execution/filter_operator.hpp"
#include "../execution/project_operator.hpp"
#include "../execution/table_scan_operator.hpp"

namespace pegasus {

std::unique_ptr<Operator>
PhysicalPlanner::Plan(const LogicalOperator &logical) const {
  switch (logical.Type()) {
  case LogicalOpType::Scan: {
    const auto &scan = static_cast<const LogicalScan &>(logical);
    return std::make_unique<TableScanOperator>(
        catalog_.GetChunk(scan.table_name));
  }
  case LogicalOpType::Filter: {
    const auto &filter = static_cast<const LogicalFilter &>(logical);
    auto child = Plan(*filter.child);
    Predicate pred = predicates::FromCompareOp(filter.op, filter.literal);
    return std::make_unique<FilterOperator>(std::move(child), filter.column_idx,
                                            std::move(pred));
  }
  case LogicalOpType::Project: {
    const auto &project = static_cast<const LogicalProject &>(logical);
    auto child = Plan(*project.child);
    return std::make_unique<ProjectOperator>(
        std::move(child), project.column_indices, project.output_names);
  }
  case LogicalOpType::Aggregate: {
    const auto &agg = static_cast<const LogicalAggregate &>(logical);
    auto child = Plan(*agg.child);
    AggrType type =
        (agg.func == AggFunc::Sum) ? AggrType::SUM : AggrType::COUNT;
    return std::make_unique<AggregateOperator>(std::move(child), agg.column_idx,
                                               type, agg.output_name);
  }
  }
  throw std::runtime_error("PhysicalPlanner: unreachable LogicalOpType");
}

} // namespace pegasus
