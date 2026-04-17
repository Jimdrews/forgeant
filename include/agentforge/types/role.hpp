#ifndef AGENTFORGE_TYPES_ROLE_HPP
#define AGENTFORGE_TYPES_ROLE_HPP

#include <cstdint>
#include <nlohmann/json.hpp>

namespace agentforge {

enum class Role : std::uint8_t { system, user, assistant, tool };

void to_json(nlohmann::json& j, Role role);
void from_json(const nlohmann::json& j, Role& role);

} // namespace agentforge

#endif // AGENTFORGE_TYPES_ROLE_HPP
