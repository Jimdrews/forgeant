#ifndef AGENTFORGE_AGENT_RESULT_HPP
#define AGENTFORGE_AGENT_RESULT_HPP

#include <agentforge/provider/response.hpp>
#include <agentforge/types/message.hpp>

#include <string>

namespace agentforge {

struct AgentResult {
    std::string text;
    Message message{Role::assistant, ""};
    Usage total_usage;
    int iterations = 0;
    std::string finish_reason;
};

} // namespace agentforge

#endif // AGENTFORGE_AGENT_RESULT_HPP
