#include <agentforge/types/role.hpp>

#include <catch2/catch_test_macros.hpp>

using agentforge::Role;

TEST_CASE("Role enum values exist", "[role]") {
    REQUIRE(Role::system != Role::user);
    REQUIRE(Role::assistant != Role::tool);
}

TEST_CASE("Role JSON round-trip", "[role]") {
    auto test = [](Role role, const std::string& expected) {
        agentforge::Json j;
        to_json(j, role);
        REQUIRE(j.get<std::string>() == expected);

        auto restored = j.get<Role>();
        REQUIRE(restored == role);
    };

    test(Role::system, "system");
    test(Role::user, "user");
    test(Role::assistant, "assistant");
    test(Role::tool, "tool");
}

TEST_CASE("Invalid role string throws", "[role]") {
    agentforge::Json j = "invalid";
    REQUIRE_THROWS_AS(j.get<Role>(), std::invalid_argument);
}
