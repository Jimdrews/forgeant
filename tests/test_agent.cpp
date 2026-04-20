#include <agentforge/agent/agent.hpp>
#include <agentforge/agent/error.hpp>
#include <agentforge/schema/schema.hpp>
#include <agentforge/tool/tool.hpp>

#include <catch2/catch_test_macros.hpp>

#include <queue>

using namespace agentforge;

struct AddParams {
    int a = 0;
    int b = 0;
};

template <>
struct agentforge::ParamSchema<AddParams> {
    static Json schema() {
        return Schema::object()
            .property("a", Schema::integer().build())
            .property("b", Schema::integer().build())
            .build();
    }
};

inline void from_json(const Json& j, AddParams& p) {
    j.at("a").get_to(p.a);
    j.at("b").get_to(p.b);
}

namespace {

class MockProvider : public LlmProvider {
  public:
    std::queue<LlmResponse> responses;
    std::vector<ChatRequest> captured_requests;

    LlmResponse chat(const Conversation&, const ChatRequest& request) override {
        captured_requests.push_back(request);
        auto r = responses.front();
        responses.pop();
        return r;
    }
};

LlmResponse text_response(const std::string& text, int input_tok = 10, int output_tok = 5) {
    LlmResponse r;
    r.message = Message(Role::assistant, text);
    r.finish_reason = "end_turn";
    r.usage = {.input_tokens = input_tok, .output_tokens = output_tok};
    r.model = "mock";
    return r;
}

LlmResponse tool_call_response(const std::string& tool_id, const std::string& tool_name,
                               const Json& input, int input_tok = 10, int output_tok = 5) {
    LlmResponse r;
    r.message = Message(Role::assistant,
                        std::vector<ContentBlock>{ToolUseBlock(tool_id, tool_name, input)});
    r.finish_reason = "tool_use";
    r.usage = {.input_tokens = input_tok, .output_tokens = output_tok};
    r.model = "mock";
    return r;
}

LlmResponse multi_tool_response(int input_tok = 10, int output_tok = 5) {
    LlmResponse r;
    r.message = Message(Role::assistant,
                        std::vector<ContentBlock>{ToolUseBlock("1", "add", {{"a", 1}, {"b", 2}}),
                                                  ToolUseBlock("2", "add", {{"a", 3}, {"b", 4}})});
    r.finish_reason = "tool_use";
    r.usage = {.input_tokens = input_tok, .output_tokens = output_tok};
    r.model = "mock";
    return r;
}

} // namespace

TEST_CASE("Agent simple text response", "[agent]") {
    MockProvider provider;
    provider.responses.push(text_response("Hello!"));

    Agent agent(provider);
    auto result = agent.run("Hi");

    REQUIRE(result.output == "Hello!");
    REQUIRE(result.iterations == 1);
    REQUIRE(result.finish_reason == "end_turn");
}

TEST_CASE("Agent tool call cycle", "[agent]") {
    MockProvider provider;
    provider.responses.push(tool_call_response("call_1", "add", {{"a", 2}, {"b", 3}}));
    provider.responses.push(text_response("The answer is 5."));

    Agent agent(provider);
    agent.add_tool(
        make_tool<AddParams>("add", "Add numbers", [](AddParams p) { return p.a + p.b; }));
    auto result = agent.run("What is 2+3?");

    REQUIRE(result.output == "The answer is 5.");
    REQUIRE(result.iterations == 2);
}

TEST_CASE("Agent multiple tool calls in one response", "[agent]") {
    MockProvider provider;
    provider.responses.push(multi_tool_response());
    provider.responses.push(text_response("Results: 3 and 7."));

    Agent agent(provider);
    agent.add_tool(
        make_tool<AddParams>("add", "Add numbers", [](AddParams p) { return p.a + p.b; }));
    auto result = agent.run("Add 1+2 and 3+4");

    REQUIRE(result.output == "Results: 3 and 7.");
    REQUIRE(result.iterations == 2);
}

TEST_CASE("Agent tool error wrapping", "[agent]") {
    MockProvider provider;
    provider.responses.push(tool_call_response("call_1", "fail", {{"a", 1}, {"b", 2}}));
    provider.responses.push(text_response("Sorry, that failed."));

    Agent agent(provider);
    agent.add_tool(Tool("fail", "Always fails", Schema::object().build(),
                        [](const Json&) -> Json { throw std::runtime_error("tool broke"); }));
    auto result = agent.run("Try the tool");

    REQUIRE(result.output == "Sorry, that failed.");
    REQUIRE(result.iterations == 2);
}

TEST_CASE("Agent max iterations throws", "[agent]") {
    MockProvider provider;
    for (int i = 0; i < 3; ++i) {
        provider.responses.push(
            tool_call_response("call_" + std::to_string(i), "add", {{"a", 1}, {"b", 1}}));
    }

    Agent agent(provider, AgentOptions{.max_iterations = 3});
    agent.add_tool(make_tool<AddParams>("add", "Add", [](AddParams p) { return p.a + p.b; }));

    try {
        agent.run("Keep adding");
        FAIL("expected AgentRunError");
    } catch (const AgentRunError& e) {
        REQUIRE(e.kind() == AgentRunError::Kind::max_iterations);
        REQUIRE(e.iterations() == 3);
        REQUIRE(e.finish_reason() == "max_iterations");
    }
}

TEST_CASE("Agent system prompt applied in run()", "[agent]") {
    MockProvider provider;
    provider.responses.push(text_response("I am helpful."));

    Agent agent(provider, AgentOptions{.system_prompt = "You are helpful."});
    auto result = agent.run("Hi");

    REQUIRE(result.output == "I am helpful.");
    REQUIRE(result.conversation.system_prompt().has_value());
    REQUIRE(*result.conversation.system_prompt() == "You are helpful.");
}

TEST_CASE("Agent multi-turn via result.conversation preserves history", "[agent]") {
    MockProvider provider;
    provider.responses.push(text_response("Hello!"));
    provider.responses.push(text_response("I'm fine."));

    Agent agent(provider);
    auto r1 = agent.run("Hi");
    REQUIRE(r1.conversation.messages().size() == 2);

    auto r2 = agent.run(r1.conversation);
    REQUIRE(r2.conversation.messages().size() == 3);
}

TEST_CASE("Agent does not mutate caller Conversation", "[agent]") {
    MockProvider provider;
    provider.responses.push(text_response("ok"));

    Agent agent(provider);

    Conversation conv;
    conv.add(Message(Role::user, "hi"));
    const Conversation snapshot = conv;

    agent.run(conv);

    REQUIRE(conv == snapshot);
}

TEST_CASE("Agent cumulative usage tracking", "[agent]") {
    MockProvider provider;
    provider.responses.push(tool_call_response("1", "add", {{"a", 1}, {"b", 2}}, 100, 50));
    provider.responses.push(text_response("Done.", 80, 30));

    Agent agent(provider);
    agent.add_tool(make_tool<AddParams>("add", "Add", [](AddParams p) { return p.a + p.b; }));
    auto result = agent.run("Add 1+2");

    REQUIRE(result.total_usage.input_tokens == 180);
    REQUIRE(result.total_usage.output_tokens == 80);
    REQUIRE(result.iterations == 2);
}

TEST_CASE("Agent provider error throws AgentRunError with partial state", "[agent]") {
    struct ErrorProvider : LlmProvider {
        LlmResponse chat(const Conversation&, const ChatRequest&) override {
            throw std::runtime_error("connection timeout");
        }
    };

    ErrorProvider provider;
    Agent agent(provider);

    try {
        agent.run("Hello");
        FAIL("expected AgentRunError");
    } catch (const AgentRunError& e) {
        REQUIRE(e.kind() == AgentRunError::Kind::provider_error);
        REQUIRE(e.finish_reason() == "error");
        REQUIRE(std::string(e.what()).find("connection timeout") != std::string::npos);
    }
}

TEST_CASE("Agent error preserves iteration count and usage", "[agent]") {
    struct ErrorAfterOneProvider : LlmProvider {
        int call_count = 0;
        LlmResponse chat(const Conversation&, const ChatRequest&) override {
            if (call_count++ == 0) {
                return tool_call_response("1", "add", {{"a", 1}, {"b", 2}}, 100, 50);
            }
            throw std::runtime_error("timeout on second call");
        }
    };

    ErrorAfterOneProvider provider;
    Agent agent(provider);
    agent.add_tool(make_tool<AddParams>("add", "Add", [](AddParams p) { return p.a + p.b; }));

    try {
        agent.run("Add");
        FAIL("expected AgentRunError");
    } catch (const AgentRunError& e) {
        REQUIRE(e.iterations() == 2);
        REQUIRE(e.usage().input_tokens == 100);
        REQUIRE(e.usage().output_tokens == 50);
    }
}

TEST_CASE("Agent RunOverrides::system_prompt applies for current call only", "[agent]") {
    MockProvider provider;
    provider.responses.push(text_response("first"));
    provider.responses.push(text_response("second"));

    Agent agent(provider, AgentOptions{.system_prompt = "stored"});
    auto r1 = agent.run("hi", RunOverrides{.system_prompt = std::string("overridden")});
    REQUIRE(*r1.conversation.system_prompt() == "overridden");

    auto r2 = agent.run("hi");
    REQUIRE(*r2.conversation.system_prompt() == "stored");
}

TEST_CASE("Agent RunOverrides::max_iterations applies for current call only", "[agent]") {
    MockProvider provider;
    for (int i = 0; i < 5; ++i) {
        provider.responses.push(
            tool_call_response("c" + std::to_string(i), "add", {{"a", 1}, {"b", 1}}));
    }

    Agent agent(provider, AgentOptions{.max_iterations = 10});
    agent.add_tool(make_tool<AddParams>("add", "Add", [](AddParams p) { return p.a + p.b; }));

    try {
        agent.run("go", RunOverrides{.max_iterations = 2});
        FAIL("expected AgentRunError");
    } catch (const AgentRunError& e) {
        REQUIRE(e.kind() == AgentRunError::Kind::max_iterations);
        REQUIRE(e.iterations() == 2);
    }
}

TEST_CASE("Conversation system_prompt wins over override", "[agent]") {
    MockProvider provider;
    provider.responses.push(text_response("ok"));

    Agent agent(provider, AgentOptions{.system_prompt = "stored"});

    Conversation conv("from conversation");
    conv.add(Message(Role::user, "hi"));

    auto r = agent.run(conv, RunOverrides{.system_prompt = std::string("override")});
    REQUIRE(*r.conversation.system_prompt() == "from conversation");
}

TEST_CASE("Agent factory invalid provider still throws", "[agent]") {
    REQUIRE_THROWS_AS(Agent::create("invalid", {.model = "x"}), std::invalid_argument);
}

TEST_CASE("Agent::run<T> composes tool loop with structured output", "[agent]") {
    MockProvider provider;
    provider.responses.push(tool_call_response("c1", "add", {{"a", 2}, {"b", 3}}));

    LlmResponse final;
    final.message = Message(Role::assistant, R"({"a": 2, "b": 3})");
    final.finish_reason = "end_turn";
    final.usage = {.input_tokens = 10, .output_tokens = 5};
    final.model = "mock";
    provider.responses.push(final);

    Agent agent(provider);
    agent.add_tool(make_tool<AddParams>("add", "Add", [](AddParams p) { return p.a + p.b; }));

    auto result = agent.run<AddParams>("Please structure as AddParams");

    REQUIRE(result.output.a == 2);
    REQUIRE(result.output.b == 3);
    REQUIRE(result.iterations == 2);

    REQUIRE(provider.captured_requests.size() >= 1);
    REQUIRE(provider.captured_requests.back().output_schema.has_value());
}
