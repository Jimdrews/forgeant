#include <agentforge/tool/registry.hpp>

#include <stdexcept>

namespace agentforge {

void ToolRegistry::add(Tool tool) {
    auto name = tool.name;
    tools_.emplace(std::move(name), std::move(tool));
}

const Tool& ToolRegistry::get(const std::string& name) const {
    auto it = tools_.find(name);
    if (it == tools_.end()) {
        throw std::runtime_error("unknown tool: " + name);
    }
    return it->second;
}

bool ToolRegistry::has(const std::string& name) const {
    return tools_.contains(name);
}

nlohmann::json ToolRegistry::execute(const std::string& name, const nlohmann::json& args) const {
    return get(name).execute(args);
}

std::vector<nlohmann::json> ToolRegistry::definitions() const {
    std::vector<nlohmann::json> defs;
    defs.reserve(tools_.size());
    for (const auto& [name, tool] : tools_) {
        defs.push_back({{"type", "function"},
                        {"function",
                         {{"name", tool.name},
                          {"description", tool.description},
                          {"parameters", tool.parameters}}}});
    }
    return defs;
}

size_t ToolRegistry::size() const {
    return tools_.size();
}

} // namespace agentforge
