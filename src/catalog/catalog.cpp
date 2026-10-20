#include "catalog.hpp"

namespace pegasus {

void Catalog::RegisterTable(const std::string &name, const Chunk &chunk) {
  tables_.insert_or_assign(name, Entry{Schema(chunk), &chunk});
}

bool Catalog::HasTable(const std::string &name) const {
  return tables_.count(name) > 0;
}

const Schema &Catalog::GetSchema(const std::string &name) const {
  auto it = tables_.find(name);
  if (it == tables_.end())
    throw std::out_of_range("Catalog: no table named '" + name + "'");
  return it->second.schema;
}

const Chunk &Catalog::GetChunk(const std::string &name) const {
  auto it = tables_.find(name);
  if (it == tables_.end())
    throw std::out_of_range("Catalog: no table named '" + name + "'");
  return *it->second.chunk;
}

} // namespace pegasus
