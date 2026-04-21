#include <agentforge/types/message.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace agentforge;

TEST_CASE("Message with single text block", "[message]") {
    Message msg(Role::assistant, std::vector<ContentBlock>{TextBlock{.text = "hello"}});

    REQUIRE(msg.role == Role::assistant);
    REQUIRE(msg.content.size() == 1);
    REQUIRE(std::holds_alternative<TextBlock>(msg.content[0]));
}

TEST_CASE("Message convenience constructor from string", "[message]") {
    Message msg(Role::user, "what is the weather?");

    REQUIRE(msg.role == Role::user);
    REQUIRE(msg.content.size() == 1);
    REQUIRE(std::get<TextBlock>(msg.content[0]).text == "what is the weather?");
}

TEST_CASE("Message with multiple content blocks", "[message]") {
    Message msg(Role::assistant,
                std::vector<ContentBlock>{
                    TextBlock{.text = "Let me check."},
                    ToolUseBlock("1", "weather", Json::object({{"city", "Denver"}}))});

    REQUIRE(msg.content.size() == 2);
    REQUIRE(std::holds_alternative<TextBlock>(msg.content[0]));
    REQUIRE(std::holds_alternative<ToolUseBlock>(msg.content[1]));
}

TEST_CASE("Message JSON round-trip", "[message]") {
    Message original(Role::assistant, "hello world");
    Json j;
    to_json(j, original);

    REQUIRE(j["role"] == "assistant");
    REQUIRE(j["content"].size() == 1);

    auto restored = j.get<Message>();
    REQUIRE(restored == original);
}
