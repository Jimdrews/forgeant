#include <agentforge/agent/error.hpp>
#include <agentforge/provider/anthropic.hpp>
#include <agentforge/provider/openai.hpp>
#include <agentforge/structured/structured.hpp>

#include <catch2/catch_test_macros.hpp>

#include "mock_http_client.hpp"

using namespace agentforge;

struct TestOutput {
    std::string name;
    int age = 0;
};

template <>
struct agentforge::ParamSchema<TestOutput> {
    static Json schema() {
        return Schema::object()
            .property("name", Schema::string().build())
            .property("age", Schema::integer().build())
            .required({"name", "age"})
            .build();
    }
};

inline void from_json(const Json& j, TestOutput& out) {
    j.at("name").get_to(out.name);
    j.at("age").get_to(out.age);
}

namespace {

testing::MockHttpClient make_anthropic_mock(const std::string& json_content, int status = 200) {
    testing::MockHttpClient mock;
    Json response_body = {{"content", {{{"type", "text"}, {"text", json_content}}}},
                          {"model", "claude-sonnet-4-20250514"},
                          {"stop_reason", "end_turn"},
                          {"usage", {{"input_tokens", 10}, {"output_tokens", 5}}}};
    mock.canned_response.status_code = status;
    mock.canned_response.body = response_body.dump();
    return mock;
}

testing::MockHttpClient make_openai_mock(const std::string& json_content, int status = 200) {
    testing::MockHttpClient mock;
    Json response_body = {{"choices",
                           {{{"message", {{"role", "assistant"}, {"content", json_content}}},
                             {"finish_reason", "stop"}}}},
                          {"model", "gpt-4o"},
                          {"usage", {{"prompt_tokens", 10}, {"completion_tokens", 5}}}};
    mock.canned_response.status_code = status;
    mock.canned_response.body = response_body.dump();
    return mock;
}

} // namespace

TEST_CASE("structured<T> with Anthropic returns typed result", "[structured]") {
    auto mock = make_anthropic_mock(R"({"name": "Alice", "age": 30})");
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Tell me about Alice"));

    auto result = structured<TestOutput>(provider, conv);
    REQUIRE(result.output.name == "Alice");
    REQUIRE(result.output.age == 30);
}

TEST_CASE("structured<T> with OpenAI returns typed result", "[structured]") {
    auto mock = make_openai_mock(R"({"name": "Bob", "age": 25})");
    ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Tell me about Bob"));

    auto result = structured<TestOutput>(provider, conv);
    REQUIRE(result.output.name == "Bob");
    REQUIRE(result.output.age == 25);
}

TEST_CASE("structured<T> extracts JSON from TextBlock", "[structured]") {
    auto mock = make_anthropic_mock(R"({"name": "Charlie", "age": 40})");
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Who?"));

    auto result = structured<TestOutput>(provider, conv);
    REQUIRE(result.output.name == "Charlie");
}

TEST_CASE("structured<T> retries on parse failure", "[structured]") {
    testing::MockHttpClient mock;
    int call_count = 0;

    Json bad_response = {{"content", {{{"type", "text"}, {"text", "not json"}}}},
                         {"model", "claude-sonnet-4-20250514"},
                         {"stop_reason", "end_turn"},
                         {"usage", {{"input_tokens", 10}, {"output_tokens", 5}}}};
    Json good_response = {
        {"content", {{{"type", "text"}, {"text", R"({"name": "Fixed", "age": 1})"}}}},
        {"model", "claude-sonnet-4-20250514"},
        {"stop_reason", "end_turn"},
        {"usage", {{"input_tokens", 10}, {"output_tokens", 5}}}};

    mock.canned_response.status_code = 200;
    mock.canned_response.body = bad_response.dump();

    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string&) {
        call_count++;
        if (call_count >= 2) {
            mock.canned_response.body = good_response.dump();
        }
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Test"));

    auto result = structured<TestOutput>(provider, conv);
    REQUIRE(result.output.name == "Fixed");
    REQUIRE(call_count >= 2);
}

TEST_CASE("structured<T> throws AgentRunError when max retries exhausted", "[structured]") {
    auto mock = make_anthropic_mock("not valid json");
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Test"));

    REQUIRE_THROWS_AS(structured<TestOutput>(provider, conv), AgentRunError);
}

TEST_CASE("structured<T> parse-exhausted error carries partial state", "[structured]") {
    auto mock = make_anthropic_mock("not valid json");
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Test"));

    try {
        structured<TestOutput>(provider, conv, {.max_retries = 1});
        FAIL("expected AgentRunError");
    } catch (const AgentRunError& e) {
        REQUIRE(e.kind() == AgentRunError::Kind::structured_parse);
        REQUIRE(e.conversation().messages().size() >= 3);
    }
}

TEST_CASE("structured<T> throws immediately with max_retries=0", "[structured]") {
    auto mock = make_anthropic_mock("not json");
    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Test"));

    REQUIRE_THROWS_AS(structured<TestOutput>(provider, conv, {.max_retries = 0}), AgentRunError);
}

TEST_CASE("structured<T> throws when response has no TextBlock", "[structured]") {
    testing::MockHttpClient mock;
    Json response_body = {
        {"content", {{{"type", "tool_use"}, {"id", "1"}, {"name", "x"}, {"input", {}}}}},
        {"model", "claude-sonnet-4-20250514"},
        {"stop_reason", "tool_use"},
        {"usage", {{"input_tokens", 10}, {"output_tokens", 5}}}};
    mock.canned_response.status_code = 200;
    mock.canned_response.body = response_body.dump();

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Test"));

    REQUIRE_THROWS_AS(structured<TestOutput>(provider, conv, {.max_retries = 0}), AgentRunError);
}

TEST_CASE("structured<T> preserves prior messages in multi-turn", "[structured]") {
    auto mock = make_anthropic_mock(R"({"name": "Alice", "age": 30})");
    Json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = Json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv("You are helpful.");
    conv.add(Message(Role::user, "First message"));
    conv.add(Message(Role::assistant, "Got it."));
    conv.add(Message(Role::user, "Now tell me about Alice"));

    auto result = structured<TestOutput>(provider, conv);
    REQUIRE(result.output.name == "Alice");
    REQUIRE(captured_body["system"] == "You are helpful.");
    REQUIRE(captured_body["messages"].size() == 3);
}

TEST_CASE("Anthropic includes output_config when schema provided via ChatRequest", "[structured]") {
    auto mock = make_anthropic_mock(R"({"name": "Test", "age": 1})");
    Json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = Json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Test"));
    provider.chat(conv, ChatRequest{.output_schema = ParamSchema<TestOutput>::schema()});

    REQUIRE(captured_body.contains("output_config"));
    REQUIRE(captured_body["output_config"]["format"]["type"] == "json_schema");
}

TEST_CASE("OpenAI includes response_format with strict when schema provided", "[structured]") {
    auto mock = make_openai_mock(R"({"name": "Test", "age": 1})");
    Json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = Json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    OpenAiProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Test"));
    provider.chat(conv, ChatRequest{.output_schema = ParamSchema<TestOutput>::schema()});

    REQUIRE(captured_body.contains("response_format"));
    REQUIRE(captured_body["response_format"]["type"] == "json_schema");
    REQUIRE(captured_body["response_format"]["json_schema"]["strict"] == true);
}

TEST_CASE("Provider accepts tools and schema together via ChatRequest", "[structured]") {
    auto mock = make_anthropic_mock(R"({"name": "Combined", "age": 1})");
    Json captured_body;
    mock.on_post = [&](const std::string&, const HttpHeaders&, const std::string& body) {
        captured_body = Json::parse(body);
    };

    ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    AnthropicProvider provider(mock, config);

    Conversation conv;
    conv.add(Message(Role::user, "Test"));

    Json noop_params = {{"type", "object"}};
    std::vector<ToolView> tool_views = {ToolView{"noop", "do nothing", noop_params}};

    provider.chat(
        conv, ChatRequest{.tools = tool_views, .output_schema = ParamSchema<TestOutput>::schema()});

    REQUIRE(captured_body.contains("tools"));
    REQUIRE(captured_body.contains("output_config"));
}
