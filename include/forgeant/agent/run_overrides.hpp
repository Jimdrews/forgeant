#ifndef FORGEANT_AGENT_RUN_OVERRIDES_HPP
#define FORGEANT_AGENT_RUN_OVERRIDES_HPP

#include <optional>
#include <string>

namespace forgeant {

/**
 * @ingroup agents
 * @brief Per-call overrides for `Agent::run()`: swap the system prompt or iteration cap for one
 * call.
 *
 * Each field is optional. An engaged `system_prompt` replaces whatever prompt the
 * agent/conversation would otherwise use for this call (leaving the agent's defaults
 * untouched). An engaged `max_iterations` replaces the agent's iteration cap for this
 * call only.
 *
 * Precedence, highest wins: `RunOverrides` > `Conversation::system_prompt` (for the
 * prompt field) > `AgentOptions::system_prompt`. The iteration cap has only two levels:
 * `RunOverrides::max_iterations` overrides `AgentOptions::max_iterations`.
 *
 * Use this for one-off behavior changes without rebuilding the agent — for example, a
 * tighter iteration cap on a specific call, or a task-specific system prompt on top of
 * a general-purpose agent.
 *
 * @see Agent::run, AgentOptions
 */
struct RunOverrides {
    /// @brief If engaged, replaces the system prompt for this call only.
    std::optional<std::string> system_prompt;
    /// @brief If engaged, replaces the iteration cap for this call only.
    std::optional<int> max_iterations;
};

} // namespace forgeant

#endif // FORGEANT_AGENT_RUN_OVERRIDES_HPP
