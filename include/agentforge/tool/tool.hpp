#ifndef AGENTFORGE_TOOL_TOOL_HPP
#define AGENTFORGE_TOOL_TOOL_HPP

#include <agentforge/schema/param_schema.hpp>

#include <functional>
#include <nlohmann/json.hpp>
#include <string>
#include <utility>

namespace agentforge {

struct Tool {
    std::string name;
    std::string description;
    nlohmann::json parameters;
    std::function<nlohmann::json(const nlohmann::json&)> execute;

    Tool(std::string name, std::string description, nlohmann::json parameters,
         std::function<nlohmann::json(const nlohmann::json&)> execute)
        : name(std::move(name)), description(std::move(description)),
          parameters(std::move(parameters)), execute(std::move(execute)) {}
};

template <typename Params, typename Func>
Tool make_tool(std::string name, std::string description, Func&& func) {
    auto schema = ParamSchema<Params>::schema();
    auto handler = [f = std::forward<Func>(func)](const nlohmann::json& args) -> nlohmann::json {
        auto params = args.get<Params>();
        return nlohmann::json(f(std::move(params)));
    };
    return Tool(std::move(name), std::move(description), std::move(schema), std::move(handler));
}

} // namespace agentforge

#endif // AGENTFORGE_TOOL_TOOL_HPP
