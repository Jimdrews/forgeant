#ifndef AGENTFORGE_TOOL_REGISTRY_HPP
#define AGENTFORGE_TOOL_REGISTRY_HPP

#include <agentforge/tool/tool.hpp>

#include <string>
#include <unordered_map>
#include <vector>

namespace agentforge {

class ToolRegistry {
  public:
    void add(Tool tool);
    [[nodiscard]] const Tool& get(const std::string& name) const;
    [[nodiscard]] bool has(const std::string& name) const;
    nlohmann::json execute(const std::string& name, const nlohmann::json& args) const;
    [[nodiscard]] std::vector<nlohmann::json> definitions() const;
    [[nodiscard]] size_t size() const;

  private:
    std::unordered_map<std::string, Tool> tools_;
};

} // namespace agentforge

#endif // AGENTFORGE_TOOL_REGISTRY_HPP
