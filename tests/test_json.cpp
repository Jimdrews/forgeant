#include <agentforge/json/json.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace agentforge;

TEST_CASE("json::parse valid JSON", "[json]") {
    auto j = json::parse(R"({"key": 42})");
    REQUIRE(j["key"] == 42);
}

TEST_CASE("json::parse invalid JSON throws", "[json]") {
    REQUIRE_THROWS(json::parse("not json"));
}

TEST_CASE("json::dump produces valid string", "[json]") {
    auto j = json::parse(R"({"key": "value"})");
    auto s = json::dump(j);
    auto roundtrip = json::parse(s);
    REQUIRE(roundtrip == j);
}

TEST_CASE("Json::get extracts string", "[json]") {
    auto j = json::parse(R"("hello")");
    REQUIRE(j.get<std::string>() == "hello");
}

TEST_CASE("Json::get extracts int", "[json]") {
    auto j = json::parse("42");
    REQUIRE(j.get<int>() == 42);
}

TEST_CASE("Json::get type mismatch throws", "[json]") {
    auto j = json::parse(R"("not a number")");
    REQUIRE_THROWS(j.get<int>());
}

TEST_CASE("Json construction from string", "[json]") {
    Json j(std::string("hello"));
    REQUIRE(j.get<std::string>() == "hello");
}

TEST_CASE("Json construction from int", "[json]") {
    Json j(42);
    REQUIRE(j.get<int>() == 42);
}

TEST_CASE("json::array returns empty array", "[json]") {
    auto j = json::array();
    REQUIRE(j.is_array());
    REQUIRE(j.empty());
}

TEST_CASE("json::object returns empty object", "[json]") {
    auto j = json::object();
    REQUIRE(j.is_object());
    REQUIRE(j.empty());
}

TEST_CASE("Json object construction with initializer list", "[json]") {
    Json j = Json::object({{"name", "test"}, {"value", 123}});
    REQUIRE(j["name"] == "test");
    REQUIRE(j["value"] == 123);
}
