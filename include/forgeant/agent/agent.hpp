#ifndef FORGEANT_AGENT_AGENT_HPP
#define FORGEANT_AGENT_AGENT_HPP

#include <exception>
#include <forgeant/agent/error.hpp>
#include <forgeant/agent/options.hpp>
#include <forgeant/agent/result.hpp>
#include <forgeant/agent/run_overrides.hpp>
#include <forgeant/http/client.hpp>
#include <forgeant/json/json.hpp>
#include <forgeant/provider/chat_request.hpp>
#include <forgeant/provider/provider.hpp>
#include <forgeant/schema/param_schema.hpp>
#include <forgeant/structured/config.hpp>
#include <forgeant/tool/registry.hpp>
#include <forgeant/types/conversation.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace forgeant {

/**
 * @ingroup agents
 * @brief The core runtime: bundles a provider, a tool registry, and the ReAct loop.
 *
 * An `Agent` owns (or borrows) an `LlmProvider`, holds a `ToolRegistry`, and drives a
 * ReAct-style loop that calls the provider, dispatches any requested tools, and feeds
 * their results back until the model produces a final answer or the iteration cap is
 * reached.
 *
 * ## Construction
 *
 * The normal path is the factory `Agent::create("anthropic" | "openai" | "ollama", opts)`,
 * which builds an HTTP client and a matching provider from `AgentOptions`. Ownership of
 * both is internal to the returned `Agent`. To plug in a custom `LlmProvider` (for tests
 * or an unsupported backend), use the `Agent(LlmProvider&, AgentOptions)` constructor
 * instead; provider-side fields on `AgentOptions` are then ignored because the provider
 * already carries its own configuration.
 *
 * ## Running
 *
 * `run(prompt)` and `run(conversation)` share the same underlying loop — the former is a
 * convenience that starts a fresh `Conversation` with a single user turn. Both accept an
 * optional `RunOverrides` to swap the system prompt or iteration cap for that call only.
 *
 * The template parameter `T` controls the output shape:
 *   - `T = std::string` (default) returns the final assistant text.
 *   - Any other `T` runs in structured mode: a JSON Schema derived from `ParamSchema<T>`
 *     is attached to the request, and the response is parsed into `T`. On parse failure
 *     the loop retries up to `StructuredConfig::max_retries` with a corrective message.
 *
 * Every `run()` returns a fresh `AgentResult<T>` including the full conversation, token
 * usage, iteration count, and finish reason — nothing is kept on the agent between calls.
 *
 * ## Reuse and threading
 *
 * An `Agent` is stateless between calls and safe to reuse across many `run()` invocations.
 * It is not thread-safe: concurrent calls on the same instance are not supported. Create
 * one `Agent` per thread, or serialize access with your own mutex.
 *
 * ## Errors
 *
 * Provider failures, iteration-cap hits, and structured-parse failures all throw
 * `AgentRunError`, which carries the conversation assembled up to the point of failure
 * along with accumulated usage. See `AgentRunError` for the contract.
 *
 * @see AgentOptions, RunOverrides, AgentResult, AgentRunError, Tool, StructuredConfig
 */
class Agent {
  public:
    /**
     * @brief Build an agent backed by one of the built-in providers.
     *
     * Supported provider names are `"anthropic"`, `"openai"`, and `"ollama"`. The Ollama
     * name reuses the OpenAI wire format and defaults `base_url` to
     * `http://localhost:11434` when unset.
     *
     * @throws std::invalid_argument if `provider` is not a recognized name.
     */
    static std::unique_ptr<Agent> create(const std::string& provider, const AgentOptions& options);

    /**
     * @brief Build an agent around a caller-owned provider (for tests or custom backends).
     *
     * Provider-side fields on `options` (`api_key`, `model`, `base_url`, `max_tokens`,
     * `temperature`) are ignored — the provided `LlmProvider` already carries its own
     * configuration. Only `system_prompt` and `max_iterations` are read.
     */
    Agent(LlmProvider& provider, AgentOptions options = {});

    /// @brief Register a tool so the model can call it during `run()`.
    void add_tool(Tool tool);

    /**
     * @brief Run the agent with a single user prompt.
     *
     * Starts a fresh `Conversation`, applies the system prompt (overrides > agent options),
     * and enters the ReAct loop. Returns once the model produces a final turn with no
     * tool calls, or throws `AgentRunError` on failure.
     *
     * @throws AgentRunError on provider failure, iteration-cap hit, or structured-parse
     *         failure (when `T` is not `std::string`).
     */
    template <typename T = std::string>
    AgentResult<T> run(std::string_view prompt, RunOverrides overrides = {}) {
        Conversation working;
        apply_system_prompt(working, overrides);
        working.add(Message(Role::user, std::string(prompt)));
        return execute<T>(std::move(working), overrides);
    }

    /**
     * @brief Run the agent with a pre-built conversation (multi-turn continuation).
     *
     * The conversation is copied before execution; the caller's copy is not mutated.
     * The returned `AgentResult::conversation` contains the full transcript including
     * the newly generated turns.
     *
     * @throws AgentRunError on provider failure, iteration-cap hit, or structured-parse
     *         failure (when `T` is not `std::string`).
     */
    template <typename T = std::string>
    AgentResult<T> run(const Conversation& conversation, RunOverrides overrides = {}) {
        Conversation working = conversation;
        apply_system_prompt(working, overrides);
        return execute<T>(std::move(working), overrides);
    }

  private:
    std::unique_ptr<HttpClient> owned_http_;
    std::unique_ptr<LlmProvider> owned_provider_;
    LlmProvider* provider_;
    ToolRegistry registry_;
    AgentOptions options_;

    Agent(std::unique_ptr<HttpClient> http, std::unique_ptr<LlmProvider> provider,
          AgentOptions options);

    struct LoopResult {
        Conversation conversation;
        Usage total_usage;
        int iterations = 0;
        std::string finish_reason;
    };

    LoopResult execute_loop(Conversation working, const RunOverrides& overrides,
                            const std::optional<Json>& output_schema);

    void apply_system_prompt(Conversation& working, const RunOverrides& overrides) const;

    static std::string extract_last_text(const Conversation& conv);

    template <typename T>
    AgentResult<T> execute(Conversation working, const RunOverrides& overrides) {
        if constexpr (std::is_same_v<T, std::string>) {
            auto loop = execute_loop(std::move(working), overrides, std::nullopt);
            AgentResult<std::string> result;
            result.output = extract_last_text(loop.conversation);
            result.total_usage = loop.total_usage;
            result.iterations = loop.iterations;
            result.finish_reason = std::move(loop.finish_reason);
            result.conversation = std::move(loop.conversation);
            return result;
        } else {
            auto schema = ParamSchema<T>::schema();
            StructuredConfig cfg;
            Usage accumulated;
            int total_iterations = 0;
            std::string last_finish_reason;
            std::string last_error;

            for (int attempt = 0; attempt <= cfg.max_retries; ++attempt) {
                auto loop = execute_loop(std::move(working), overrides, schema);
                working = std::move(loop.conversation);
                accumulated.input_tokens += loop.total_usage.input_tokens;
                accumulated.output_tokens += loop.total_usage.output_tokens;
                total_iterations += loop.iterations;
                last_finish_reason = std::move(loop.finish_reason);

                auto text = extract_last_text(working);
                try {
                    auto json = Json::parse(text);
                    AgentResult<T> result;
                    result.output = json.template get<T>();
                    result.conversation = std::move(working);
                    result.total_usage = accumulated;
                    result.iterations = total_iterations;
                    result.finish_reason = std::move(last_finish_reason);
                    return result;
                } catch (const std::exception& e) {
                    last_error = e.what();
                    if (attempt < cfg.max_retries) {
                        working.add(Message(
                            Role::user,
                            "Your response did not match the required schema: " + last_error +
                                ". Please respond with valid JSON matching the schema."));
                    }
                }
            }

            throw AgentRunError(
                AgentRunError::Kind::structured_parse,
                "structured output failed after " + std::to_string(cfg.max_retries + 1) +
                    " attempts: " + last_error,
                std::move(working), accumulated, total_iterations, std::move(last_finish_reason));
        }
    }
};

} // namespace forgeant

#endif // FORGEANT_AGENT_AGENT_HPP
