#pragma once

#include <agentforge/types/content.hpp>
#include <agentforge/types/role.hpp>

#include <string>
#include <vector>

namespace agentforge {

struct Message {
    Role role = Role::user;
    std::vector<ContentBlock> content;

    Message() = default;

    Message(Role role, std::vector<ContentBlock> content)
        : role(role), content(std::move(content)) {}

    Message(Role role, std::string text) : role(role), content{TextBlock{std::move(text)}} {}

    bool operator==(const Message&) const = default;
};

void to_json(nlohmann::json& j, const Message& msg);
void from_json(const nlohmann::json& j, Message& msg);

} // namespace agentforge
