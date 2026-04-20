#ifndef AGENTFORGE_AGENT_RUN_OVERRIDES_HPP
#define AGENTFORGE_AGENT_RUN_OVERRIDES_HPP

#include <optional>
#include <string>

namespace agentforge {

struct RunOverrides {
    std::optional<std::string> system_prompt;
    std::optional<int> max_iterations;
};

} // namespace agentforge

#endif // AGENTFORGE_AGENT_RUN_OVERRIDES_HPP
