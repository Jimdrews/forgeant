#ifndef AGENTFORGE_TOOL_TOOL_VIEW_HPP
#define AGENTFORGE_TOOL_TOOL_VIEW_HPP

#include <nlohmann/json.hpp>
#include <string_view>

namespace agentforge {

struct ToolView {
    std::string_view name;
    std::string_view description;
    const nlohmann::json& parameters;
};

} // namespace agentforge

#endif // AGENTFORGE_TOOL_TOOL_VIEW_HPP
