#include <agentforge/version.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("version_string returns non-empty string", "[version]") {
    auto version = agentforge::version_string();
    REQUIRE(!version.empty());
}

TEST_CASE("version constants are defined", "[version]") {
    REQUIRE(agentforge::VERSION_MAJOR >= 0);
    REQUIRE(agentforge::VERSION_MINOR >= 0);
    REQUIRE(agentforge::VERSION_PATCH >= 0);
}

TEST_CASE("version_string matches constants", "[version]") {
    auto version = agentforge::version_string();
    auto expected = std::to_string(agentforge::VERSION_MAJOR) + "." +
                    std::to_string(agentforge::VERSION_MINOR) + "." +
                    std::to_string(agentforge::VERSION_PATCH);
    REQUIRE(version == expected);
}
