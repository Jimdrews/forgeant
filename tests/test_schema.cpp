#include <agentforge/schema/param_schema.hpp>
#include <agentforge/schema/schema.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace agentforge;

TEST_CASE("Schema::string() produces string type", "[schema]") {
    auto s = Schema::string().build();
    REQUIRE(s["type"] == "string");
}

TEST_CASE("Schema::integer() produces integer type", "[schema]") {
    auto s = Schema::integer().build();
    REQUIRE(s["type"] == "integer");
}

TEST_CASE("Schema::number() produces number type", "[schema]") {
    auto s = Schema::number().build();
    REQUIRE(s["type"] == "number");
}

TEST_CASE("Schema::boolean() produces boolean type", "[schema]") {
    auto s = Schema::boolean().build();
    REQUIRE(s["type"] == "boolean");
}

TEST_CASE("Schema builder chains description", "[schema]") {
    auto s = Schema::string().description("The city name").build();
    REQUIRE(s["type"] == "string");
    REQUIRE(s["description"] == "The city name");
}

TEST_CASE("Schema builder chains default_value", "[schema]") {
    auto s = Schema::string().default_value("fahrenheit").build();
    REQUIRE(s["default"] == "fahrenheit");
}

TEST_CASE("Schema builder chains enum_values", "[schema]") {
    auto s = Schema::string().enum_values({"fahrenheit", "celsius"}).build();
    REQUIRE(s["enum"].size() == 2);
    REQUIRE(s["enum"][0] == "fahrenheit");
    REQUIRE(s["enum"][1] == "celsius");
}

TEST_CASE("Schema::object() with properties", "[schema]") {
    auto s = Schema::object()
                 .property("name", Schema::string().build())
                 .property("age", Schema::integer().build())
                 .build();
    REQUIRE(s["type"] == "object");
    REQUIRE(s["properties"]["name"]["type"] == "string");
    REQUIRE(s["properties"]["age"]["type"] == "integer");
}

TEST_CASE("Schema::object() with required fields", "[schema]") {
    auto s = Schema::object().property("name", Schema::string().build()).required({"name"}).build();
    REQUIRE(s["required"].size() == 1);
    REQUIRE(s["required"][0] == "name");
}

TEST_CASE("Schema::array() with items", "[schema]") {
    auto s = Schema::array().items(Schema::string().build()).build();
    REQUIRE(s["type"] == "array");
    REQUIRE(s["items"]["type"] == "string");
}

TEST_CASE("ParamSchema<std::string> returns string schema", "[schema]") {
    auto s = ParamSchema<std::string>::schema();
    REQUIRE(s["type"] == "string");
}

TEST_CASE("ParamSchema<int> returns integer schema", "[schema]") {
    auto s = ParamSchema<int>::schema();
    REQUIRE(s["type"] == "integer");
}

TEST_CASE("ParamSchema<double> returns number schema", "[schema]") {
    auto s = ParamSchema<double>::schema();
    REQUIRE(s["type"] == "number");
}

TEST_CASE("ParamSchema<bool> returns boolean schema", "[schema]") {
    auto s = ParamSchema<bool>::schema();
    REQUIRE(s["type"] == "boolean");
}
