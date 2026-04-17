#include <agentforge/types/conversation.hpp>

namespace agentforge {

void to_json(nlohmann::json& j, const Conversation& conv) {
    j = {{"messages", nlohmann::json::array()}};
    if (conv.system_prompt().has_value()) {
        j["system_prompt"] = conv.system_prompt().value();
    }
    for (const auto& msg : conv.messages()) {
        j["messages"].push_back(msg);
    }
}

void from_json(const nlohmann::json& j, Conversation& conv) {
    if (j.contains("system_prompt")) {
        conv.set_system_prompt(j.at("system_prompt").get<std::string>());
    }
    for (const auto& msg_json : j.at("messages")) {
        conv.add(msg_json.get<Message>());
    }
}

} // namespace agentforge
