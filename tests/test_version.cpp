#include <catch2/catch_test_macros.hpp>

#include <forgeant/version.hpp>

TEST_CASE("version_string returns non-empty string", "[version]") {
    auto version = forgeant::version_string();
    REQUIRE(!version.empty());
}

TEST_CASE("version constants are defined", "[version]") {
    REQUIRE(forgeant::VERSION_MAJOR >= 0);
    REQUIRE(forgeant::VERSION_MINOR >= 0);
    REQUIRE(forgeant::VERSION_PATCH >= 0);
}

TEST_CASE("version_string matches constants", "[version]") {
    auto version = forgeant::version_string();
    auto expected = std::to_string(forgeant::VERSION_MAJOR) + "." +
                    std::to_string(forgeant::VERSION_MINOR) + "." +
                    std::to_string(forgeant::VERSION_PATCH);
    REQUIRE(version == expected);
}
