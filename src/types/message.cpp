#include <agentforge/types/message.hpp>

namespace agentforge {

void to_json(Json& j, const Message& msg) {
    j = {{"role", msg.role}, {"content", msg.content}};
}

void from_json(const Json& j, Message& msg) {
    j.at("role").get_to(msg.role);
    j.at("content").get_to(msg.content);
}

} // namespace agentforge
