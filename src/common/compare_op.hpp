
#pragma once

namespace pegasus {

enum class CompareOp { Eq, Neq, Lt, Gt, Lte, Gte };

inline const char *ToString(CompareOp op) {
  switch (op) {
  case CompareOp::Eq:
    return "=";
  case CompareOp::Neq:
    return "!=";
  case CompareOp::Lt:
    return "<";
  case CompareOp::Gt:
    return ">";
  case CompareOp::Lte:
    return "<=";
  case CompareOp::Gte:
    return ">=";
  }
  return "?";
}

} // namespace pegasus
