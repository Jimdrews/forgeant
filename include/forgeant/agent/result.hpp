#ifndef FORGEANT_AGENT_RESULT_HPP
#define FORGEANT_AGENT_RESULT_HPP

#include <forgeant/provider/response.hpp>
#include <forgeant/types/conversation.hpp>
#include <string>

namespace forgeant {

/**
 * @ingroup agents
 * @brief Return value of `Agent::run()`: the parsed output plus the full transcript, usage, and
 * metadata.
 */
template <typename T = std::string>
struct AgentResult {
    T output{};
    Conversation conversation;
    Usage total_usage;
    int iterations = 0;
    std::string finish_reason;
};

} // namespace forgeant

#endif // FORGEANT_AGENT_RESULT_HPP
