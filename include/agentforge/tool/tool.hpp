#ifndef AGENTFORGE_TOOL_TOOL_HPP
#define AGENTFORGE_TOOL_TOOL_HPP

#include <agentforge/json/json.hpp>
#include <agentforge/schema/param_schema.hpp>

#include <functional>
#include <string>
#include <utility>

namespace agentforge {

struct Tool {
    std::string name;
    std::string description;
    Json parameters;
    std::function<Json(const Json&)> execute;

    Tool(std::string name, std::string description, Json parameters,
         std::function<Json(const Json&)> execute)
        : name(std::move(name)), description(std::move(description)),
          parameters(std::move(parameters)), execute(std::move(execute)) {}
};

template <typename Params, typename Func>
Tool make_tool(std::string name, std::string description, Func&& func) {
    auto schema = ParamSchema<Params>::schema();
    auto handler = [f = std::forward<Func>(func)](const Json& args) -> Json {
        auto params = args.get<Params>();
        return Json(f(std::move(params)));
    };
    return Tool(std::move(name), std::move(description), std::move(schema), std::move(handler));
}

} // namespace agentforge

#endif // AGENTFORGE_TOOL_TOOL_HPP
