#ifndef FORGEANT_TYPES_CONVERSATION_HPP
#define FORGEANT_TYPES_CONVERSATION_HPP

#include <forgeant/types/message.hpp>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace forgeant {

/**
 * @ingroup types
 * @brief An ordered sequence of messages plus an optional system prompt.
 */
class Conversation {
  public:
    Conversation() = default;
    explicit Conversation(std::string system_prompt) : system_prompt_(std::move(system_prompt)) {}

    void set_system_prompt(std::string prompt) { system_prompt_ = std::move(prompt); }
    [[nodiscard]] const std::optional<std::string>& system_prompt() const { return system_prompt_; }

    void add(Message message) { messages_.push_back(std::move(message)); }
    [[nodiscard]] std::span<const Message> messages() const { return messages_; }

    void clear() { messages_.clear(); }

    bool operator==(const Conversation&) const = default;

  private:
    std::optional<std::string> system_prompt_;
    std::vector<Message> messages_;
};

void to_json(Json& j, const Conversation& conv);
void from_json(const Json& j, Conversation& conv);

} // namespace forgeant

#endif // FORGEANT_TYPES_CONVERSATION_HPP
