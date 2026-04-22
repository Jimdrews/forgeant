#ifndef FORGEANT_TYPES_MESSAGE_HPP
#define FORGEANT_TYPES_MESSAGE_HPP

#include <forgeant/types/content.hpp>
#include <forgeant/types/role.hpp>
#include <string>
#include <vector>

namespace forgeant {

struct Message {
    Role role = Role::user;
    std::vector<ContentBlock> content;

    Message() = default;

    Message(Role role, std::vector<ContentBlock> content)
        : role(role), content(std::move(content)) {}

    Message(Role role, std::string text) : role(role), content{TextBlock{std::move(text)}} {}

    bool operator==(const Message&) const = default;
};

void to_json(Json& j, const Message& msg);
void from_json(const Json& j, Message& msg);

} // namespace forgeant

#endif // FORGEANT_TYPES_MESSAGE_HPP
