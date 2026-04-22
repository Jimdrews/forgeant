#include <forgeant/types/conversation.hpp>

namespace forgeant {

void to_json(Json& j, const Conversation& conv) {
    j = Json::object({{"messages", Json::array()}});
    if (const auto& sp = conv.system_prompt()) {
        j["system_prompt"] = *sp;
    }
    for (const auto& msg : conv.messages()) {
        Json msg_json;
        to_json(msg_json, msg);
        j["messages"].push_back(std::move(msg_json));
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

} // namespace forgeant
