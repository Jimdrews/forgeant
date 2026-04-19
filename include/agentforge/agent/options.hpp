#ifndef AGENTFORGE_AGENT_OPTIONS_HPP
#define AGENTFORGE_AGENT_OPTIONS_HPP

#include <optional>
#include <string>

namespace agentforge {

struct AgentOptions {
    std::string api_key;
    std::string model;
    std::string base_url;
    std::string system_prompt;
    int max_iterations = 10;
    std::optional<int> max_tokens;
    std::optional<double> temperature;
};

} // namespace agentforge

#endif // AGENTFORGE_AGENT_OPTIONS_HPP
