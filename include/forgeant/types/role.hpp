#ifndef FORGEANT_TYPES_ROLE_HPP
#define FORGEANT_TYPES_ROLE_HPP

#include <cstdint>
#include <forgeant/json/json.hpp>

namespace forgeant {

/**
 * @ingroup types
 * @brief The author of a message: system prompt, user input, assistant reply, or tool result.
 */
enum class Role : std::uint8_t { system, user, assistant, tool };

void to_json(Json& j, Role role);
void from_json(const Json& j, Role& role);

} // namespace forgeant

#endif // FORGEANT_TYPES_ROLE_HPP
