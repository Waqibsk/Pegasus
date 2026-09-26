#pragma once 

#include <cstdint>
#include <string>
#include <variant>

namespace pegasus { 

	struct NullType { 
		bool operator == (const NullType&) const { return true; }
	}

	enum class TypeId:: uint8_t {
		Integer,
		Double,
		Varchar,
	}

	using Value = std::variant<NullType, int64_t, double, std::string>;

}















