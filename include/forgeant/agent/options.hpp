#ifndef FORGEANT_AGENT_OPTIONS_HPP
#define FORGEANT_AGENT_OPTIONS_HPP

#include <optional>
#include <string>

namespace forgeant {

// Single agent-facing configuration struct.
//
// Used by `Agent::create()` (factory path) and the public `Agent(LlmProvider&, AgentOptions)`
// constructor. Fields split by ownership:
//
//   Provider-side: api_key, model, base_url, max_tokens, temperature
//   Agent-side:    system_prompt, max_iterations
//
// When passed to `Agent::create()`, all fields are consumed (provider-side forwarded to a
// derived ProviderConfig, agent-side retained on the Agent).
//
// When passed to the public `Agent(LlmProvider&, AgentOptions)` constructor, provider-side
// fields are IGNORED — the pre-built provider already owns its own configuration. Only
// system_prompt and max_iterations are read.
struct AgentOptions {
    std::string api_key;
    std::string model;
    std::string base_url;
    std::string system_prompt;
    int max_iterations = 10;
    std::optional<int> max_tokens;
    std::optional<double> temperature;
};

} // namespace forgeant

#endif // FORGEANT_AGENT_OPTIONS_HPP
