#include <agentforge/types/conversation.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace agentforge;

TEST_CASE("Conversation default has no system prompt", "[conversation]") {
    Conversation conv;
    REQUIRE(!conv.system_prompt().has_value());
    REQUIRE(conv.messages().empty());
}

TEST_CASE("Conversation with system prompt", "[conversation]") {
    Conversation conv("You are a helpful assistant.");
    REQUIRE(conv.system_prompt().has_value());
    REQUIRE(conv.system_prompt().value() == "You are a helpful assistant.");
}

TEST_CASE("Conversation message ordering", "[conversation]") {
    Conversation conv;
    conv.add(Message(Role::user, "first"));
    conv.add(Message(Role::assistant, "second"));
    conv.add(Message(Role::user, "third"));

    REQUIRE(conv.messages().size() == 3);
    REQUIRE(std::get<TextBlock>(conv.messages()[0].content[0]).text == "first");
    REQUIRE(std::get<TextBlock>(conv.messages()[1].content[0]).text == "second");
    REQUIRE(std::get<TextBlock>(conv.messages()[2].content[0]).text == "third");
}

TEST_CASE("Conversation clear preserves system prompt", "[conversation]") {
    Conversation conv("system prompt");
    conv.add(Message(Role::user, "hello"));
    conv.clear();

    REQUIRE(conv.messages().empty());
    REQUIRE(conv.system_prompt().has_value());
    REQUIRE(conv.system_prompt().value() == "system prompt");
}

TEST_CASE("Conversation JSON round-trip", "[conversation]") {
    Conversation original("You are helpful.");
    original.add(Message(Role::user, "hello"));
    original.add(Message(Role::assistant, "hi there"));

    Json j;
    to_json(j, original);

    REQUIRE(j["system_prompt"] == "You are helpful.");
    REQUIRE(j["messages"].size() == 2);

    Conversation restored;
    from_json(j, restored);
    REQUIRE(restored == original);
}

TEST_CASE("Conversation JSON round-trip without system prompt", "[conversation]") {
    Conversation original;
    original.add(Message(Role::user, "hello"));

    Json j;
    to_json(j, original);
    REQUIRE(!j.contains("system_prompt"));

    Conversation restored;
    from_json(j, restored);
    REQUIRE(restored == original);
}
