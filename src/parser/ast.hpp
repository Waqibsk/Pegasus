#pragma once

#include <optional>
#include <string>
#include <variant>
#include <vector>

#include "../common/compare_op.hpp"
#include "../common/value.hpp"

namespace pegasus {

enum class AggFunc { Sum, Count };

struct ColumnRefExpr {
  std::string column_name;
};

struct AggregateExpr {
  AggFunc func;
  std::string column_name;
};

using SelectItem = std::variant<ColumnRefExpr, AggregateExpr>;

struct WhereClause {
  std::string column_name;
  CompareOp op;
  Value literal;
};

struct SelectStmt {
  bool select_star = false;
  std::vector<SelectItem> items;
  std::string table_name;
  std::optional<WhereClause> where;
};

} // namespace pegasus
