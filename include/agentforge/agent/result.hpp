#ifndef AGENTFORGE_AGENT_RESULT_HPP
#define AGENTFORGE_AGENT_RESULT_HPP

#include <agentforge/provider/response.hpp>
#include <agentforge/types/conversation.hpp>

#include <string>

namespace agentforge {

template <typename T = std::string>
struct AgentResult {
    T output{};
    Conversation conversation;
    Usage total_usage;
    int iterations = 0;
    std::string finish_reason;
};

} // namespace agentforge

#endif // AGENTFORGE_AGENT_RESULT_HPP
