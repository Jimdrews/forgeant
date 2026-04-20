#include <agentforge/agent/error.hpp>

#include <utility>

namespace agentforge {

AgentRunError::AgentRunError(Kind kind, const std::string& message, Conversation conversation,
                             Usage usage, int iterations, std::string finish_reason)
    : std::runtime_error(message), kind_(kind), conversation_(std::move(conversation)),
      usage_(usage), iterations_(iterations), finish_reason_(std::move(finish_reason)) {}

} // namespace agentforge
