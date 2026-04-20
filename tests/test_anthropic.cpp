#include <agentforge/provider/anthropic.hpp>

#include <catch2/catch_test_macros.hpp>

#include "mock_http_client.hpp"

using namespace agentforge;

namespace {

testing::MockHttpClient make_mock(const std::string& response_body, int status = 200) {
    testing::MockHttpClient mock;
    mock.canned_response.status_code = status;
    mock.canned_response.body = response_body;
    return mock;
}

const std::string SIMPLE_RESPONSE = R"({
    "content": [{"type": "text", "text": "Hello!"}],
    "model": "claude-sonnet-4-20250514",
    "stop_reason": "end_turn",
    "usage": {"input_tokens": 10, "output_tokens": 5}
})";

const std::string TOOL_USE_RESPONSE = R"({
    "content": [
        {"type": "text", "text": "Let me check."},
        {"type": "tool_use", "id": "call_1", "name": "get_weather", "input": {"city": "Denver"}}
    ],
    "model": "claude-sonnet-4-20250514",
    "stop_reason": "tool_use",
    "usage": {"input_tokens": 20, "output_tokens": 15}
})";

} // namespace

TEST_CASE("Anthropic serializes system prompt as top-level param", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "test-key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv("You are helpful.");
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE(captured_body.contains("system"));
    REQUIRE(captured_body["system"] == "You are helpful.");
    REQUIRE(captured_body["messages"].size() == 1);
}

TEST_CASE("Anthropic serializes content as block arrays", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hello"));
    provider.chat(conv);

    auto& content = captured_body["messages"][0]["content"];
    REQUIRE(content.is_array());
    REQUIRE(content[0]["type"] == "text");
    REQUIRE(content[0]["text"] == "Hello");
}

TEST_CASE("Anthropic maps Role::tool to user", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::tool, std::vector<ContentBlock>{
                                     ToolResultBlock{.tool_use_id = "call_1", .content = "72F"}}));
    provider.chat(conv);

    REQUIRE(captured_body["messages"][0]["role"] == "user");
}

TEST_CASE("Anthropic includes required fields", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE(captured_body.contains("model"));
    REQUIRE(captured_body.contains("messages"));
    REQUIRE(captured_body.contains("max_tokens"));
}

TEST_CASE("Anthropic deserializes text response", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    auto response = provider.chat(conv);

    REQUIRE(response.message.role == Role::assistant);
    REQUIRE(response.message.content.size() == 1);
    REQUIRE(std::holds_alternative<TextBlock>(response.message.content[0]));
    REQUIRE(std::get<TextBlock>(response.message.content[0]).text == "Hello!");
}

TEST_CASE("Anthropic deserializes tool_use response", "[anthropic]") {
    auto mock = make_mock(TOOL_USE_RESPONSE);
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "What's the weather?"));
    auto response = provider.chat(conv);

    REQUIRE(response.message.content.size() == 2);
    REQUIRE(std::holds_alternative<TextBlock>(response.message.content[0]));
    REQUIRE(std::holds_alternative<ToolUseBlock>(response.message.content[1]));
    auto& tool = std::get<ToolUseBlock>(response.message.content[1]);
    REQUIRE(tool.name == "get_weather");
    REQUIRE(tool.input["city"] == "Denver");
}

TEST_CASE("Anthropic parses usage", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    auto response = provider.chat(conv);

    REQUIRE(response.usage.input_tokens == 10);
    REQUIRE(response.usage.output_tokens == 5);
    REQUIRE(response.finish_reason == "end_turn");
}

TEST_CASE("Anthropic sends auth headers", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    HttpHeaders captured_headers;
    mock.on_post = [&](const std::string&, const HttpHeaders& headers, const std::string&) {
        captured_headers = headers;
    };

    ProviderConfig config{.api_key = "sk-test-123", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE(captured_headers["x-api-key"] == "sk-test-123");
    REQUIRE(captured_headers.contains("anthropic-version"));
    REQUIRE(captured_headers["content-type"] == "application/json");
}

TEST_CASE("Anthropic uses default base URL", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    std::string captured_url;
    mock.on_post = [&](const std::string& url, const HttpHeaders&, const std::string&) {
        captured_url = url;
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE(captured_url == "https://api.anthropic.com/v1/messages");
}

TEST_CASE("Anthropic serializes tools in Anthropic wire shape", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    nlohmann::json weather_params = {{"type", "object"},
                                     {"properties", {{"city", {{"type", "string"}}}}},
                                     {"required", nlohmann::json::array({"city"})}};
    std::vector<ToolView> tool_views = {
        ToolView{"get_weather", "Get the weather for a city", weather_params}};

    Conversation conv;
    conv.add(Message(Role::user, "Weather?"));
    provider.chat(conv, ChatRequest{.tools = tool_views});

    REQUIRE(captured_body.contains("tools"));
    REQUIRE(captured_body["tools"].size() == 1);
    const auto& tool = captured_body["tools"][0];
    REQUIRE(tool["name"] == "get_weather");
    REQUIRE(tool["description"] == "Get the weather for a city");
    REQUIRE(tool["input_schema"] == weather_params);
    REQUIRE_FALSE(tool.contains("type"));
    REQUIRE_FALSE(tool.contains("function"));
}

TEST_CASE("Anthropic omits tools field when empty", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE_FALSE(captured_body.contains("tools"));
}

TEST_CASE("Anthropic output_config places schema directly under format", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    nlohmann::json schema = {{"type", "object"}, {"properties", {{"name", {{"type", "string"}}}}}};

    Conversation conv;
    conv.add(Message(Role::user, "Give me JSON"));
    provider.chat(conv, ChatRequest{.output_schema = schema});

    REQUIRE(captured_body.contains("output_config"));
    const auto& format = captured_body["output_config"]["format"];
    REQUIRE(format["type"] == "json_schema");
    REQUIRE(format["schema"]["type"] == "object");
    REQUIRE(format["schema"]["additionalProperties"] == false);
    REQUIRE_FALSE(format.contains("json_schema"));
    REQUIRE_FALSE(format.contains("name"));
}

TEST_CASE("Anthropic leaves non-object schemas unchanged", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    nlohmann::json schema = {{"type", "string"}};

    Conversation conv;
    conv.add(Message(Role::user, "Give me JSON"));
    provider.chat(conv, ChatRequest{.output_schema = schema});

    const auto& emitted = captured_body["output_config"]["format"]["schema"];
    REQUIRE(emitted["type"] == "string");
    REQUIRE_FALSE(emitted.contains("additionalProperties"));
}

TEST_CASE("Anthropic does not mutate caller's schema", "[anthropic]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    nlohmann::json schema = {{"type", "object"}, {"properties", {{"name", {{"type", "string"}}}}}};
    nlohmann::json schema_before = schema;

    Conversation conv;
    conv.add(Message(Role::user, "Give me JSON"));
    provider.chat(conv, ChatRequest{.output_schema = schema});

    REQUIRE(schema == schema_before);
}

TEST_CASE("Anthropic throws on error response", "[anthropic]") {
    auto mock = make_mock(
        R"({"error": {"type": "invalid_request_error", "message": "max_tokens required"}})", 400);
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    REQUIRE_THROWS_AS(provider.chat(conv), std::runtime_error);
}
