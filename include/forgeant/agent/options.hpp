#ifndef FORGEANT_AGENT_OPTIONS_HPP
#define FORGEANT_AGENT_OPTIONS_HPP

#include <optional>
#include <string>

namespace forgeant {

/**
 * @ingroup agents
 * @brief Bundled configuration for `Agent::create()` and the public `Agent` constructor.
 *
 * Fields split by ownership:
 *   - **Provider-side:** `api_key`, `model`, `base_url`, `max_tokens`, `temperature`.
 *   - **Agent-side:** `system_prompt`, `max_iterations`.
 *
 * When passed to `Agent::create()`, all fields are consumed — provider-side fields are
 * forwarded to a derived `ProviderConfig` and agent-side fields are retained on the
 * `Agent`. When passed to the `Agent(LlmProvider&, AgentOptions)` constructor instead,
 * provider-side fields are **ignored** because the caller-owned provider already carries
 * its own configuration; only `system_prompt` and `max_iterations` are read.
 *
 * ## Per-provider notes
 *
 * - **Anthropic:** `api_key` required, `base_url` defaults to the public API, `model` is
 *   a Claude model ID (e.g., `claude-sonnet-4-20250514`).
 * - **OpenAI:** `api_key` required, `base_url` defaults to the public API, `model` is an
 *   OpenAI chat model.
 * - **Ollama:** `api_key` is typically empty, `base_url` defaults to
 *   `http://localhost:11434` when unset, `model` is a locally-pulled model tag.
 *
 * `system_prompt` is applied to the conversation if the conversation does not already
 * carry one; it can be overridden per-call via `RunOverrides`. `max_iterations` is the
 * ReAct-loop cap; exceeding it throws `AgentRunError` with kind `max_iterations`.
 *
 * @see Agent::create, ProviderConfig, RunOverrides
 */
struct AgentOptions {
    /// @brief Credential forwarded to the provider (ignored when using a custom provider).
    std::string api_key;
    /// @brief Model identifier; format depends on the provider.
    std::string model;
    /// @brief Overrides the provider's default base URL; empty means "use the default".
    std::string base_url;
    /// @brief Default system prompt applied when the conversation does not supply one.
    std::string system_prompt;
    /// @brief Maximum ReAct-loop iterations before `AgentRunError` is thrown.
    int max_iterations = 10;
    /// @brief Optional token cap forwarded to the provider.
    std::optional<int> max_tokens;
    /// @brief Optional sampling temperature forwarded to the provider.
    std::optional<double> temperature;
};

} // namespace forgeant

#endif // FORGEANT_AGENT_OPTIONS_HPP
