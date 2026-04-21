#include <agentforge/types/message.hpp>

namespace agentforge {

void to_json(Json& j, const Message& msg) {
    j = Json::object();
    to_json(j["role"], msg.role);
    Json content_arr = Json::array();
    for (const auto& block : msg.content) {
        Json block_json;
        to_json(block_json, block);
        content_arr.push_back(std::move(block_json));
    }
    j["content"] = std::move(content_arr);
}

void from_json(const Json& j, Message& msg) {
    j.at("role").get_to(msg.role);
    msg.content.clear();
    for (const auto& block_json : j.at("content")) {
        msg.content.push_back(block_json.get<ContentBlock>());
    }
}

} // namespace agentforge
