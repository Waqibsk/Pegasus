#pragma once
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "../common/chunk.hpp"
#include "schema.hpp"

namespace pegasus {

class Catalog {
public:
  void RegisterTable(const std::string &name, const Chunk &chunk);

  bool HasTable(const std::string &name) const;
  const Schema &GetSchema(const std::string &name) const;
  const Chunk &GetChunk(const std::string &name) const;

private:
  struct Entry {
    Schema schema;
    const Chunk *chunk;
  };
  std::unordered_map<std::string, Entry> tables_;
};

} // namespace pegasus
