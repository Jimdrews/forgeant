#pragma once

#include <agentforge/types/message.hpp>

#include <optional>
#include <span>
#include <string>
#include <vector>

namespace agentforge {

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

void to_json(nlohmann::json& j, const Conversation& conv);
void from_json(const nlohmann::json& j, Conversation& conv);

} // namespace agentforge
