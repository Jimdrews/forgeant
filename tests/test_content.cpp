#include <agentforge/types/content.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace agentforge;

TEST_CASE("TextBlock creation and equality", "[content]") {
    TextBlock a{.text = "hello"};
    TextBlock b{.text = "hello"};
    TextBlock c{.text = "world"};

    REQUIRE(a == b);
    REQUIRE(a != c);
}

TEST_CASE("TextBlock JSON round-trip", "[content]") {
    TextBlock original{.text = "hello world"};
    Json j = original;

    REQUIRE(j["type"] == "text");
    REQUIRE(j["text"] == "hello world");

    auto restored = j.get<TextBlock>();
    REQUIRE(restored == original);
}

TEST_CASE("ToolUseBlock JSON round-trip", "[content]") {
    ToolUseBlock original("call_123", "get_weather", {{"city", "Denver"}});
    Json j = original;

    REQUIRE(j["type"] == "tool_use");
    REQUIRE(j["id"] == "call_123");
    REQUIRE(j["name"] == "get_weather");

    auto restored = j.get<ToolUseBlock>();
    REQUIRE(restored == original);
}

TEST_CASE("ToolResultBlock defaults is_error to false", "[content]") {
    ToolResultBlock block{.tool_use_id = "call_123", .content = "72F"};
    REQUIRE(block.is_error == false);
}

TEST_CASE("ToolResultBlock JSON round-trip", "[content]") {
    ToolResultBlock original{
        .tool_use_id = "call_123", .content = "error: not found", .is_error = true};
    Json j = original;

    REQUIRE(j["type"] == "tool_result");
    REQUIRE(j["is_error"] == true);

    auto restored = j.get<ToolResultBlock>();
    REQUIRE(restored == original);
}

TEST_CASE("ContentBlock variant holds correct alternative", "[content]") {
    ContentBlock text = TextBlock{.text = "hi"};
    ContentBlock tool_use = ToolUseBlock("1", "test", Json::object());
    ContentBlock tool_result = ToolResultBlock{.tool_use_id = "1", .content = "ok"};

    REQUIRE(std::holds_alternative<TextBlock>(text));
    REQUIRE(std::holds_alternative<ToolUseBlock>(tool_use));
    REQUIRE(std::holds_alternative<ToolResultBlock>(tool_result));
}

TEST_CASE("ContentBlock JSON round-trip preserves variant type", "[content]") {
    ContentBlock original = ToolUseBlock("1", "test", {{"a", 1}});
    Json j = original;

    REQUIRE(j["type"] == "tool_use");

    ContentBlock restored = j.get<ContentBlock>();
    REQUIRE(std::holds_alternative<ToolUseBlock>(restored));
    REQUIRE(std::get<ToolUseBlock>(restored) == std::get<ToolUseBlock>(original));
}

TEST_CASE("ContentBlock visit works", "[content]") {
    ContentBlock block = TextBlock{.text = "hello"};
    std::string result = std::visit(
        [](const auto& b) -> std::string {
            using T = std::decay_t<decltype(b)>;
            if constexpr (std::is_same_v<T, TextBlock>) {
                return b.text;
            } else {
                return "not text";
            }
        },
        block);
    REQUIRE(result == "hello");
}
