#ifndef AGENTFORGE_TOOL_REGISTRY_HPP
#define AGENTFORGE_TOOL_REGISTRY_HPP

#include <agentforge/tool/tool.hpp>
#include <agentforge/tool/tool_view.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace agentforge {

class ToolRegistry {
  public:
    void add(Tool tool);
    [[nodiscard]] const Tool& get(const std::string& name) const;
    [[nodiscard]] bool has(const std::string& name) const;
    Json execute(const std::string& name, const Json& args) const;
    [[nodiscard]] std::vector<ToolView> tools() const;
    [[nodiscard]] size_t size() const;

  private:
    std::unordered_map<std::string, Tool> tools_;
};

} // namespace agentforge

#endif // AGENTFORGE_TOOL_REGISTRY_HPP
