#include <agentforge/types/conversation.hpp>

namespace agentforge {

void to_json(Json& j, const Conversation& conv) {
    j = {{"messages", Json::array()}};
    if (const auto& sp = conv.system_prompt()) {
        j["system_prompt"] = *sp;
    }
    for (const auto& msg : conv.messages()) {
        j["messages"].push_back(msg);
    }
}

void from_json(const Json& j, Conversation& conv) {
    if (j.contains("system_prompt")) {
        conv.set_system_prompt(j.at("system_prompt").get<std::string>());
    }
    for (const auto& msg_json : j.at("messages")) {
        conv.add(msg_json.get<Message>());
    }
}

} // namespace agentforge
