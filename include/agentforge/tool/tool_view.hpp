#ifndef AGENTFORGE_TOOL_TOOL_VIEW_HPP
#define AGENTFORGE_TOOL_TOOL_VIEW_HPP

#include <agentforge/json/json.hpp>

#include <string_view>

namespace agentforge {

struct ToolView {
    std::string_view name;
    std::string_view description;
    const Json& parameters;
};

} // namespace agentforge

#endif // AGENTFORGE_TOOL_TOOL_VIEW_HPP
