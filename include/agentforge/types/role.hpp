#pragma once

#include <nlohmann/json.hpp>

namespace agentforge {

enum class Role { system, user, assistant, tool };

void to_json(nlohmann::json& j, Role role);
void from_json(const nlohmann::json& j, Role& role);

} // namespace agentforge
