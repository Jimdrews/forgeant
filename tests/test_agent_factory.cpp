#include <agentforge/agent/agent.hpp>
#include <agentforge/agent/options.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace agentforge;

TEST_CASE("Agent::create throws on invalid provider", "[factory]") {
    AgentOptions options{.api_key = "key", .model = "model"};
    REQUIRE_THROWS_AS(Agent::create("unknown", options), std::invalid_argument);
}

TEST_CASE("Agent::create throws on empty provider name", "[factory]") {
    AgentOptions options{.api_key = "key", .model = "model"};
    REQUIRE_THROWS_AS(Agent::create("", options), std::invalid_argument);
}

TEST_CASE("Agent::create accepts anthropic", "[factory]") {
    AgentOptions options{.api_key = "test-key", .model = "claude-sonnet-4-20250514"};
    auto agent = Agent::create("anthropic", options);
    REQUIRE(agent != nullptr);
}

TEST_CASE("Agent::create accepts openai", "[factory]") {
    AgentOptions options{.api_key = "test-key", .model = "gpt-4o"};
    auto agent = Agent::create("openai", options);
    REQUIRE(agent != nullptr);
}

TEST_CASE("Agent::create accepts ollama", "[factory]") {
    AgentOptions options{.model = "llama3"};
    auto agent = Agent::create("ollama", options);
    REQUIRE(agent != nullptr);
}

TEST_CASE("Agent::create with custom base_url", "[factory]") {
    AgentOptions options{.model = "custom-model", .base_url = "http://localhost:9999"};
    auto agent = Agent::create("openai", options);
    REQUIRE(agent != nullptr);
}

TEST_CASE("Agent::create with system prompt", "[factory]") {
    AgentOptions options{.api_key = "key", .model = "model", .system_prompt = "Be helpful."};
    auto agent = Agent::create("anthropic", options);
    REQUIRE(agent != nullptr);
}

TEST_CASE("Agent::create with tools", "[factory]") {
    AgentOptions options{.api_key = "key", .model = "model"};
    auto agent = Agent::create("anthropic", options);
    agent->add_tool(Tool("test", "A test tool", Schema::object().build(),
                         [](const nlohmann::json&) -> nlohmann::json { return "ok"; }));
    REQUIRE(agent != nullptr);
}

TEST_CASE("Factory agent destruction cleans up owned resources", "[factory]") {
    {
        AgentOptions options{.api_key = "key", .model = "model"};
        auto agent = Agent::create("anthropic", options);
        REQUIRE(agent != nullptr);
    }
}

TEST_CASE("AgentOptions defaults are applied with minimal config", "[factory]") {
    AgentOptions options{.api_key = "key", .model = "model"};
    REQUIRE(options.max_iterations == 10);
    REQUIRE(options.base_url.empty());
    REQUIRE(options.system_prompt.empty());
    REQUIRE(!options.max_tokens.has_value());
    REQUIRE(!options.temperature.has_value());

    auto agent = Agent::create("anthropic", options);
    REQUIRE(agent != nullptr);
}

TEST_CASE("AgentOptions with all fields set", "[factory]") {
    AgentOptions options{
        .api_key = "sk-test",
        .model = "claude-sonnet-4-20250514",
        .base_url = "https://custom.api.com",
        .system_prompt = "You are helpful.",
        .max_iterations = 5,
        .max_tokens = 2048,
        .temperature = 0.7,
    };

    auto agent = Agent::create("anthropic", options);
    REQUIRE(agent != nullptr);
}

TEST_CASE("Factory agent run() works with explicit provider mock", "[factory]") {
    struct MockProvider : LlmProvider {
        LlmResponse chat(const Conversation&) override {
            LlmResponse r;
            r.message = Message(Role::assistant, "factory response");
            r.finish_reason = "end_turn";
            return r;
        }
        LlmResponse chat(const Conversation&, std::span<const nlohmann::json>) override {
            return chat(Conversation{});
        }
        LlmResponse chat(const Conversation&, const nlohmann::json&) override {
            return chat(Conversation{});
        }
    };

    MockProvider provider;
    Agent agent(provider);
    auto result = agent.run("test");
    REQUIRE(result.text == "factory response");
}

TEST_CASE("Explicit Agent constructor still works", "[factory]") {
    struct MinimalProvider : LlmProvider {
        LlmResponse chat(const Conversation&) override {
            LlmResponse r;
            r.message = Message(Role::assistant, "hi");
            r.finish_reason = "end_turn";
            return r;
        }
        LlmResponse chat(const Conversation&, std::span<const nlohmann::json>) override {
            return chat(Conversation{});
        }
        LlmResponse chat(const Conversation&, const nlohmann::json&) override {
            return chat(Conversation{});
        }
    };

    MinimalProvider provider;
    Agent agent(provider, {.system_prompt = "test"});
    auto result = agent.run("hello");
    REQUIRE(result.text == "hi");
}
