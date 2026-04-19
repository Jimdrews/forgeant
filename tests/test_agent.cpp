#include <agentforge/agent/agent.hpp>
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
    static nlohmann::json schema() {
        return Schema::object()
            .property("a", Schema::integer().build())
            .property("b", Schema::integer().build())
            .build();
    }
};

inline void from_json(const nlohmann::json& j, AddParams& p) {
    j.at("a").get_to(p.a);
    j.at("b").get_to(p.b);
}

namespace {

class MockProvider : public LlmProvider {
  public:
    std::queue<LlmResponse> responses;
    std::vector<nlohmann::json> captured_requests;

    LlmResponse chat(const Conversation& conversation) override { return next_response(); }

    LlmResponse chat(const Conversation& conversation,
                     std::span<const nlohmann::json> tools) override {
        return next_response();
    }

    LlmResponse chat(const Conversation& conversation,
                     const nlohmann::json& output_schema) override {
        return next_response();
    }

  private:
    LlmResponse next_response() {
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
                               const nlohmann::json& input, int input_tok = 10,
                               int output_tok = 5) {
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

    REQUIRE(result.text == "Hello!");
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

    REQUIRE(result.text == "The answer is 5.");
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

    REQUIRE(result.text == "Results: 3 and 7.");
    REQUIRE(result.iterations == 2);
}

TEST_CASE("Agent tool error wrapping", "[agent]") {
    MockProvider provider;
    provider.responses.push(tool_call_response("call_1", "fail", {{"a", 1}, {"b", 2}}));
    provider.responses.push(text_response("Sorry, that failed."));

    Agent agent(provider);
    agent.add_tool(Tool(
        "fail", "Always fails", Schema::object().build(),
        [](const nlohmann::json&) -> nlohmann::json { throw std::runtime_error("tool broke"); }));
    auto result = agent.run("Try the tool");

    REQUIRE(result.text == "Sorry, that failed.");
    REQUIRE(result.iterations == 2);
}

TEST_CASE("Agent max iterations termination", "[agent]") {
    MockProvider provider;
    for (int i = 0; i < 3; ++i) {
        provider.responses.push(
            tool_call_response("call_" + std::to_string(i), "add", {{"a", 1}, {"b", 1}}));
    }

    Agent agent(provider, {.max_iterations = 3});
    agent.add_tool(make_tool<AddParams>("add", "Add", [](AddParams p) { return p.a + p.b; }));
    auto result = agent.run("Keep adding");

    REQUIRE(result.iterations == 3);
    REQUIRE(result.finish_reason == "max_iterations");
}

TEST_CASE("Agent system prompt applied in run()", "[agent]") {
    MockProvider provider;
    provider.responses.push(text_response("I am helpful."));

    Agent agent(provider, {.system_prompt = "You are helpful."});
    auto result = agent.run("Hi");

    REQUIRE(result.text == "I am helpful.");
}

TEST_CASE("Agent multi-turn chat preserves history", "[agent]") {
    MockProvider provider;
    provider.responses.push(text_response("Hello!"));
    provider.responses.push(text_response("I'm fine."));

    Agent agent(provider);
    Conversation conv;
    agent.chat(conv, "Hi");

    REQUIRE(conv.messages().size() == 2);

    agent.chat(conv, "How are you?");

    REQUIRE(conv.messages().size() == 4);
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

TEST_CASE("Agent provider error returns error result", "[agent]") {
    struct ErrorProvider : LlmProvider {
        LlmResponse chat(const Conversation&) override {
            throw std::runtime_error("connection timeout");
        }
        LlmResponse chat(const Conversation&, std::span<const nlohmann::json>) override {
            throw std::runtime_error("connection timeout");
        }
        LlmResponse chat(const Conversation&, const nlohmann::json&) override {
            throw std::runtime_error("connection timeout");
        }
    };

    ErrorProvider provider;
    Agent agent(provider);
    auto result = agent.run("Hello");

    REQUIRE(result.has_error());
    REQUIRE(result.finish_reason == "error");
    REQUIRE(result.error.find("connection timeout") != std::string::npos);
}

TEST_CASE("Agent error preserves iteration count and usage", "[agent]") {
    MockProvider provider;
    provider.responses.push(tool_call_response("1", "add", {{"a", 1}, {"b", 2}}, 100, 50));

    struct ErrorAfterOneProvider : LlmProvider {
        MockProvider& inner;
        int call_count = 0;
        explicit ErrorAfterOneProvider(MockProvider& m) : inner(m) {}
        LlmResponse chat(const Conversation& c) override {
            if (call_count++ > 0) {
                throw std::runtime_error("timeout on second call");
            }
            return inner.chat(c);
        }
        LlmResponse chat(const Conversation& c, std::span<const nlohmann::json> t) override {
            if (call_count++ > 0) {
                throw std::runtime_error("timeout on second call");
            }
            return inner.chat(c, t);
        }
        LlmResponse chat(const Conversation&, const nlohmann::json&) override {
            throw std::runtime_error("not used");
        }
    };

    ErrorAfterOneProvider error_provider(provider);
    Agent agent(error_provider);
    agent.add_tool(make_tool<AddParams>("add", "Add", [](AddParams p) { return p.a + p.b; }));
    auto result = agent.run("Add");

    REQUIRE(result.has_error());
    REQUIRE(result.iterations == 2);
    REQUIRE(result.total_usage.input_tokens == 100);
    REQUIRE(result.total_usage.output_tokens == 50);
}

TEST_CASE("Agent successful run has no error", "[agent]") {
    MockProvider provider;
    provider.responses.push(text_response("Hello!"));

    Agent agent(provider);
    auto result = agent.run("Hi");

    REQUIRE(!result.has_error());
    REQUIRE(result.error.empty());
}

TEST_CASE("Agent factory invalid provider still throws", "[agent]") {
    REQUIRE_THROWS_AS(Agent::create("invalid", {.model = "x"}), std::invalid_argument);
}
