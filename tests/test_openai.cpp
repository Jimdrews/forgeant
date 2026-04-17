#include <agentforge/provider/openai.hpp>

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
    "choices": [{
        "message": {"role": "assistant", "content": "Hello!"},
        "finish_reason": "stop"
    }],
    "model": "gpt-4o",
    "usage": {"prompt_tokens": 10, "completion_tokens": 5}
})";

const std::string TOOL_CALL_RESPONSE = R"({
    "choices": [{
        "message": {
            "role": "assistant",
            "content": null,
            "tool_calls": [{
                "id": "call_1",
                "type": "function",
                "function": {"name": "get_weather", "arguments": "{\"city\":\"Denver\"}"}
            }]
        },
        "finish_reason": "tool_calls"
    }],
    "model": "gpt-4o",
    "usage": {"prompt_tokens": 20, "completion_tokens": 15}
})";

} // namespace

TEST_CASE("OpenAI serializes system prompt as system message", "[openai]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "test-key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv("You are helpful.");
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE(captured_body["messages"][0]["role"] == "system");
    REQUIRE(captured_body["messages"][0]["content"] == "You are helpful.");
    REQUIRE(captured_body["messages"][1]["role"] == "user");
}

TEST_CASE("OpenAI serializes text content as string", "[openai]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hello"));
    provider.chat(conv);

    REQUIRE(captured_body["messages"][0]["content"].is_string());
    REQUIRE(captured_body["messages"][0]["content"] == "Hello");
}

TEST_CASE("OpenAI serializes tool result with tool role", "[openai]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::tool, std::vector<ContentBlock>{
                                     ToolResultBlock{.tool_use_id = "call_1", .content = "72F"}}));
    provider.chat(conv);

    REQUIRE(captured_body["messages"][0]["role"] == "tool");
    REQUIRE(captured_body["messages"][0]["tool_call_id"] == "call_1");
    REQUIRE(captured_body["messages"][0]["content"] == "72F");
}

TEST_CASE("OpenAI includes required fields", "[openai]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    nlohmann::json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = nlohmann::json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE(captured_body.contains("model"));
    REQUIRE(captured_body.contains("messages"));
}

TEST_CASE("OpenAI deserializes text response", "[openai]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    auto response = provider.chat(conv);

    REQUIRE(response.message.role == Role::assistant);
    REQUIRE(response.message.content.size() == 1);
    REQUIRE(std::get<TextBlock>(response.message.content[0]).text == "Hello!");
}

TEST_CASE("OpenAI deserializes tool_calls response", "[openai]") {
    auto mock = make_mock(TOOL_CALL_RESPONSE);
    ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "What's the weather?"));
    auto response = provider.chat(conv);

    REQUIRE(response.message.content.size() == 1);
    REQUIRE(std::holds_alternative<ToolUseBlock>(response.message.content[0]));
    auto& tool = std::get<ToolUseBlock>(response.message.content[0]);
    REQUIRE(tool.name == "get_weather");
    REQUIRE(tool.input["city"] == "Denver");
}

TEST_CASE("OpenAI maps usage fields correctly", "[openai]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    auto response = provider.chat(conv);

    REQUIRE(response.usage.input_tokens == 10);
    REQUIRE(response.usage.output_tokens == 5);
    REQUIRE(response.finish_reason == "stop");
}

TEST_CASE("OpenAI sends Bearer auth header", "[openai]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    HttpHeaders captured_headers;
    mock.on_post = [&](const std::string&, const HttpHeaders& headers, const std::string&) {
        captured_headers = headers;
    };

    ProviderConfig config{.api_key = "sk-test-123", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE(captured_headers["Authorization"] == "Bearer sk-test-123");
    REQUIRE(captured_headers["content-type"] == "application/json");
}

TEST_CASE("OpenAI uses default base URL", "[openai]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    std::string captured_url;
    mock.on_post = [&](const std::string& url, const HttpHeaders&, const std::string&) {
        captured_url = url;
    };

    ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE(captured_url == "https://api.openai.com/v1/chat/completions");
}

TEST_CASE("OpenAI uses custom base URL for compatible endpoints", "[openai]") {
    auto mock = make_mock(SIMPLE_RESPONSE);
    std::string captured_url;
    mock.on_post = [&](const std::string& url, const HttpHeaders&, const std::string&) {
        captured_url = url;
    };

    ProviderConfig config{.api_key = "", .model = "llama3", .base_url = "http://localhost:11434"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    provider.chat(conv);

    REQUIRE(captured_url == "http://localhost:11434/v1/chat/completions");
}

TEST_CASE("OpenAI throws on error response", "[openai]") {
    auto mock = make_mock(
        R"({"error": {"message": "Invalid API key", "type": "invalid_request_error"}})", 401);
    ProviderConfig config{.api_key = "bad-key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Hi"));
    REQUIRE_THROWS_AS(provider.chat(conv), std::runtime_error);
}
