#include <forgeant/types/role.hpp>
#include <stdexcept>
#include <string>

namespace forgeant {

void to_json(Json& j, Role role) {
    switch (role) {
    case Role::system:
        j = "system";
        break;
    case Role::user:
        j = "user";
        break;
    case Role::assistant:
        j = "assistant";
        break;
    case Role::tool:
        j = "tool";
        break;
    }
}

void from_json(const Json& j, Role& role) {
    auto s = j.get<std::string>();
    if (s == "system") {
        role = Role::system;
    } else if (s == "user") {
        role = Role::user;
    } else if (s == "assistant") {
        role = Role::assistant;
    } else if (s == "tool") {
        role = Role::tool;
    } else {
        throw std::invalid_argument("invalid role: " + s);
    }
}

} // namespace forgeant
