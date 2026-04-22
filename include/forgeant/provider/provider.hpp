#ifndef FORGEANT_PROVIDER_PROVIDER_HPP
#define FORGEANT_PROVIDER_PROVIDER_HPP

#include <forgeant/provider/chat_request.hpp>
#include <forgeant/provider/response.hpp>
#include <forgeant/types/conversation.hpp>

namespace forgeant {

class LlmProvider {
  public:
    virtual ~LlmProvider() = default;

    virtual LlmResponse chat(const Conversation& conversation, const ChatRequest& request = {}) = 0;

    LlmProvider() = default;
    LlmProvider(const LlmProvider&) = delete;
    LlmProvider& operator=(const LlmProvider&) = delete;
    LlmProvider(LlmProvider&&) = default;
    LlmProvider& operator=(LlmProvider&&) = default;
};

} // namespace forgeant

#endif // FORGEANT_PROVIDER_PROVIDER_HPP
