#ifndef FORGEANT_AGENT_ERROR_HPP
#define FORGEANT_AGENT_ERROR_HPP

#include <cstdint>
#include <forgeant/provider/response.hpp>
#include <forgeant/types/conversation.hpp>
#include <stdexcept>
#include <string>

namespace forgeant {

/**
 * @ingroup agents
 * @brief Thrown by `Agent::run()` when the loop cannot produce a result; preserves partial
 * progress.
 *
 * Every `AgentRunError` carries the conversation as it stood when the failure occurred,
 * the accumulated token usage, how many loop iterations ran, and the last finish reason
 * the provider reported. That means callers can log the partial transcript, resume from
 * it, or surface it for debugging — the conversation is not lost to the exception.
 *
 * ## Kinds
 *
 * - `provider_error` — the underlying `LlmProvider::chat()` threw (network failure,
 *   rate limit, bad credentials, serialization error). The original message is in
 *   `what()`.
 * - `max_iterations` — the agent hit the iteration cap without a terminal turn. Common
 *   when a tool-call loop stalls. Raise `AgentOptions::max_iterations` or inspect the
 *   conversation to see why.
 * - `structured_parse` — for `Agent::run<T>()`, the final attempt still could not be
 *   parsed into `T` after `StructuredConfig::max_retries`. The conversation includes
 *   every failed response and corrective user turn.
 *
 * ## Usage
 *
 * @code
 * try {
 *     auto result = agent->run("…");
 * } catch (const forgeant::AgentRunError& e) {
 *     std::cerr << e.what() << " (kind=" << static_cast<int>(e.kind())
 *               << ", iters=" << e.iterations() << ")\n";
 *     // e.conversation() contains the partial transcript
 * }
 * @endcode
 *
 * @see Agent, AgentOptions, StructuredConfig
 */
class AgentRunError : public std::runtime_error {
  public:
    /// @brief Categorizes why the run failed; see class docs for the contract of each value.
    enum class Kind : std::uint8_t {
        provider_error,
        max_iterations,
        structured_parse,
    };

    AgentRunError(Kind kind, const std::string& message, Conversation conversation, Usage usage,
                  int iterations, std::string finish_reason);

    /// @brief The failure category.
    [[nodiscard]] Kind kind() const noexcept { return kind_; }
    /// @brief The conversation as it stood when the failure occurred.
    [[nodiscard]] const Conversation& conversation() const noexcept { return conversation_; }
    /// @brief Token usage accumulated up to the point of failure.
    [[nodiscard]] const Usage& usage() const noexcept { return usage_; }
    /// @brief Number of loop iterations that completed before failure.
    [[nodiscard]] int iterations() const noexcept { return iterations_; }
    /// @brief Last finish reason reported by the provider, or `"error"` on provider failure.
    [[nodiscard]] const std::string& finish_reason() const noexcept { return finish_reason_; }

  private:
    Kind kind_;
    Conversation conversation_;
    Usage usage_;
    int iterations_;
    std::string finish_reason_;
};

} // namespace forgeant

#endif // FORGEANT_AGENT_ERROR_HPP
