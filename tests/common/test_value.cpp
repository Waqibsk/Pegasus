#include <catch2/catch_test_macros.hpp>

#include "common/value.hpp"

using namespace pegasus;

TEST_CASE("Value holds each alternative correctly", "[value]") {
  Value int_val{int64_t{42}};
  Value double_val{3.14};
  Value str_val{std::string("hello")};
  Value null_val{NullType{}};

  SECTION("integer") {
    REQUIRE(std::holds_alternative<int64_t>(int_val));
    REQUIRE(std::get<int64_t>(int_val) == 42);
    REQUIRE(TypeIdOf(int_val) == TypeId::Integer);
    REQUIRE_FALSE(IsNull(int_val));
  }

  SECTION("double") {
    REQUIRE(std::holds_alternative<double>(double_val));
    REQUIRE(std::get<double>(double_val) == 3.14);
    REQUIRE(TypeIdOf(double_val) == TypeId::Double);
  }

  SECTION("string") {
    REQUIRE(std::holds_alternative<std::string>(str_val));
    REQUIRE(std::get<std::string>(str_val) == "hello");
    REQUIRE(TypeIdOf(str_val) == TypeId::Varchar);
  }

  SECTION("null") {
    REQUIRE(IsNull(null_val));
    REQUIRE_THROWS_AS(TypeIdOf(null_val), std::runtime_error);
  }
}

TEST_CASE("ToString produces readable type names", "[value]") {
  REQUIRE(std::string(ToString(TypeId::Integer)) == "INTEGER");
  REQUIRE(std::string(ToString(TypeId::Double)) == "DOUBLE");
  REQUIRE(std::string(ToString(TypeId::Varchar)) == "VARCHAR");
}
