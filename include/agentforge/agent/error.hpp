#ifndef AGENTFORGE_AGENT_ERROR_HPP
#define AGENTFORGE_AGENT_ERROR_HPP

#include <agentforge/provider/response.hpp>
#include <agentforge/types/conversation.hpp>

#include <cstdint>
#include <stdexcept>
#include <string>

namespace agentforge {

class AgentRunError : public std::runtime_error {
  public:
    enum class Kind : std::uint8_t {
        provider_error,
        max_iterations,
        structured_parse,
    };

    AgentRunError(Kind kind, const std::string& message, Conversation conversation, Usage usage,
                  int iterations, std::string finish_reason);

    [[nodiscard]] Kind kind() const noexcept { return kind_; }
    [[nodiscard]] const Conversation& conversation() const noexcept { return conversation_; }
    [[nodiscard]] const Usage& usage() const noexcept { return usage_; }
    [[nodiscard]] int iterations() const noexcept { return iterations_; }
    [[nodiscard]] const std::string& finish_reason() const noexcept { return finish_reason_; }

  private:
    Kind kind_;
    Conversation conversation_;
    Usage usage_;
    int iterations_;
    std::string finish_reason_;
};

} // namespace agentforge

#endif // AGENTFORGE_AGENT_ERROR_HPP
