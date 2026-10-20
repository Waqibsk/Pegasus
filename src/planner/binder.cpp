#include "binder.hpp"

#include <variant>

namespace pegasus {

namespace {

bool LiteralMatchesColumnType(TypeId col_type, const Value &literal) {
  if (col_type == TypeId::Varchar)
    return std::holds_alternative<std::string>(literal);
  return std::holds_alternative<int64_t>(literal) ||
         std::holds_alternative<double>(literal);
}

std::string DefaultAggOutputName(AggFunc func, const std::string &column) {
  return (func == AggFunc::Sum ? std::string("sum_") : std::string("count_")) +
         column;
}

} // namespace

std::unique_ptr<LogicalOperator> Binder::Bind(const SelectStmt &stmt) const {
  if (!catalog_.HasTable(stmt.table_name)) {
    throw BindError("unknown table '" + stmt.table_name + "'");
  }
  const Schema &schema = catalog_.GetSchema(stmt.table_name);

  std::unique_ptr<LogicalOperator> plan =
      std::make_unique<LogicalScan>(stmt.table_name);

  if (stmt.where.has_value()) {
    const WhereClause &where = *stmt.where;
    if (!schema.HasColumn(where.column_name)) {
      throw BindError("WHERE references unknown column '" + where.column_name +
                      "' on table '" + stmt.table_name + "'");
    }
    size_t col_idx = schema.IndexOf(where.column_name);
    TypeId col_type = schema.Columns()[col_idx].type;
    if (!LiteralMatchesColumnType(col_type, where.literal)) {
      throw BindError("type mismatch in WHERE clause: column '" +
                      where.column_name + "' is " + ToString(col_type) +
                      " but the literal is not");
    }
    plan = std::make_unique<LogicalFilter>(std::move(plan), col_idx, where.op,
                                           where.literal);
  }

  if (stmt.select_star) {
    std::vector<size_t> indices;
    std::vector<std::string> names;
    for (size_t i = 0; i < schema.NumColumns(); ++i) {
      indices.push_back(i);
      names.push_back(schema.Columns()[i].name);
    }
    return std::make_unique<LogicalProject>(std::move(plan), std::move(indices),
                                            std::move(names));
  }

  std::vector<ColumnRefExpr> columns;
  std::vector<AggregateExpr> aggregates;
  for (const SelectItem &item : stmt.items) {
    std::visit(
        [&](const auto &expr) {
          using T = std::decay_t<decltype(expr)>;
          if constexpr (std::is_same_v<T, ColumnRefExpr>) {
            columns.push_back(expr);
          } else {
            aggregates.push_back(expr);
          }
        },
        item);
  }

  if (!aggregates.empty() && !columns.empty()) {
    throw BindError(
        "cannot mix aggregate and non-aggregate columns in the SELECT list "
        "(this engine has no GROUP BY)");
  }
  if (aggregates.size() > 1) {
    throw BindError(
        "only a single aggregate expression per query is supported");
  }

  if (!aggregates.empty()) {
    const AggregateExpr &agg = aggregates.front();
    if (!schema.HasColumn(agg.column_name)) {
      throw BindError("aggregate references unknown column '" +
                      agg.column_name + "' on table '" + stmt.table_name + "'");
    }
    size_t col_idx = schema.IndexOf(agg.column_name);
    return std::make_unique<LogicalAggregate>(
        std::move(plan), col_idx, agg.func,
        DefaultAggOutputName(agg.func, agg.column_name));
  }

  std::vector<size_t> indices;
  std::vector<std::string> names;
  for (const ColumnRefExpr &col : columns) {
    if (!schema.HasColumn(col.column_name)) {
      throw BindError("SELECT references unknown column '" + col.column_name +
                      "' on table '" + stmt.table_name + "'");
    }
    indices.push_back(schema.IndexOf(col.column_name));
    names.push_back(col.column_name);
  }
  return std::make_unique<LogicalProject>(std::move(plan), std::move(indices),
                                          std::move(names));
}

} // namespace pegasus
