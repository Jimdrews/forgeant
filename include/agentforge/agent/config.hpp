#ifndef AGENTFORGE_AGENT_CONFIG_HPP
#define AGENTFORGE_AGENT_CONFIG_HPP

#include <string>

namespace agentforge {

struct AgentConfig {
    int max_iterations = 10;
    std::string system_prompt;
};

} // namespace agentforge

#endif // AGENTFORGE_AGENT_CONFIG_HPP
