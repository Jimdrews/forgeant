#ifndef AGENTFORGE_PROVIDER_PROVIDER_HPP
#define AGENTFORGE_PROVIDER_PROVIDER_HPP

#include <agentforge/provider/response.hpp>
#include <agentforge/types/conversation.hpp>

namespace agentforge {

class LlmProvider {
  public:
    virtual ~LlmProvider() = default;

    virtual LlmResponse chat(const Conversation& conversation) = 0;

    LlmProvider() = default;
    LlmProvider(const LlmProvider&) = delete;
    LlmProvider& operator=(const LlmProvider&) = delete;
    LlmProvider(LlmProvider&&) = default;
    LlmProvider& operator=(LlmProvider&&) = default;
};

} // namespace agentforge

#endif // AGENTFORGE_PROVIDER_PROVIDER_HPP
