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

Json ToolRegistry::execute(const std::string& name, const Json& args) const {
    return get(name).execute(args);
}

std::vector<ToolView> ToolRegistry::tools() const {
    std::vector<ToolView> views;
    views.reserve(tools_.size());
    for (const auto& [name, tool] : tools_) {
        views.push_back(ToolView{
            .name = tool.name, .description = tool.description, .parameters = tool.parameters});
    }
    return views;
}

size_t ToolRegistry::size() const {
    return tools_.size();
}

} // namespace agentforge
