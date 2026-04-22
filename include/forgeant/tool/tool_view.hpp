#ifndef FORGEANT_TOOL_TOOL_VIEW_HPP
#define FORGEANT_TOOL_TOOL_VIEW_HPP

#include <forgeant/json/json.hpp>
#include <string_view>

namespace forgeant {

struct ToolView {
    std::string_view name;
    std::string_view description;
    const Json& parameters;
};

} // namespace forgeant

#endif // FORGEANT_TOOL_TOOL_VIEW_HPP
