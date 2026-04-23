#ifndef FORGEANT_TOOL_REGISTRY_HPP
#define FORGEANT_TOOL_REGISTRY_HPP

#include <forgeant/tool/tool.hpp>
#include <forgeant/tool/tool_view.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace forgeant {

/**
 * @ingroup tools
 * @brief Name-keyed collection of `Tool` instances; the agent dispatches tool calls through this.
 */
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

} // namespace forgeant

#endif // FORGEANT_TOOL_REGISTRY_HPP
