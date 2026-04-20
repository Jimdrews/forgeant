#ifndef AGENTFORGE_TYPES_ROLE_HPP
#define AGENTFORGE_TYPES_ROLE_HPP

#include <agentforge/json/json.hpp>

#include <cstdint>

namespace agentforge {

enum class Role : std::uint8_t { system, user, assistant, tool };

void to_json(Json& j, Role role);
void from_json(const Json& j, Role& role);

} // namespace agentforge

#endif // AGENTFORGE_TYPES_ROLE_HPP
