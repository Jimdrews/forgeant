#ifndef FORGEANT_TOOL_TOOL_HPP
#define FORGEANT_TOOL_TOOL_HPP

#include <forgeant/json/json.hpp>
#include <forgeant/schema/param_schema.hpp>
#include <functional>
#include <string>
#include <utility>

namespace forgeant {

/**
 * @ingroup tools
 * @brief A callable capability exposed to the model: name, description, JSON Schema, and handler.
 *
 * A `Tool` is what the model sees: a name and description (surfaced to the LLM so it
 * knows when to call it), a JSON Schema describing its parameters (surfaced as the tool
 * definition on the wire), and an `execute` function that the agent invokes when the
 * model issues a matching tool call. The function receives the model-supplied arguments
 * as `Json` and returns its result as `Json`, which is fed back into the conversation.
 *
 * Tools are registered on an agent with `Agent::add_tool()`. The registry is consulted
 * during the ReAct loop — any tool-use block the model produces is matched by `name`.
 *
 * ## Prefer `make_tool`
 *
 * Constructing a `Tool` directly requires hand-writing a JSON Schema and a
 * `Json`-to-`Json` lambda. `make_tool<Params>(name, description, func)` is almost always
 * the right choice: it derives the schema from `ParamSchema<Params>`, then wraps `func`
 * so it is called with a deserialized `Params` value and its return is re-serialized to
 * `Json`. Use the direct constructor only when you genuinely need a dynamic schema or a
 * raw `Json` handler.
 *
 * ## Exceptions from handlers
 *
 * Exceptions thrown inside the handler propagate out of the agent's `run()` call as
 * `AgentRunError` (kind `provider_error`), with the partially assembled conversation
 * preserved on the error. If you want the model to see a failure and try again, catch
 * inside the handler and return a `Json` describing the error instead.
 *
 * @see make_tool, ToolRegistry, ParamSchema, Agent::add_tool
 */
struct Tool {
    /// @brief The tool name shown to the model; must be unique within a registry.
    std::string name;
    /// @brief Human-readable description the model uses to decide when to call this tool.
    std::string description;
    /// @brief JSON Schema describing the expected arguments object.
    Json parameters;
    /// @brief Invoked by the agent when the model issues a matching tool call.
    std::function<Json(const Json&)> execute;

    Tool(std::string name, std::string description, Json parameters,
         std::function<Json(const Json&)> execute)
        : name(std::move(name)), description(std::move(description)),
          parameters(std::move(parameters)), execute(std::move(execute)) {}
};

/**
 * @ingroup tools
 * @brief Build a `Tool` from a typed parameter struct and a regular function.
 *
 * Derives the JSON Schema from `ParamSchema<Params>` and wraps `func` so it is called
 * with a deserialized `Params` value. The function's return type must be convertible
 * to `Json` (either directly, via a `to_json` overload, or by returning a `Json`).
 *
 * `Params` must have a `ParamSchema<Params>` specialization — the primitive types are
 * built in; for user types, specialize `ParamSchema` and provide a `from_json` overload.
 *
 * @see ParamSchema, Tool
 */
template <typename Params, typename Func>
Tool make_tool(std::string name, std::string description, Func&& func) {
    auto schema = ParamSchema<Params>::schema();
    auto handler = [f = std::forward<Func>(func)](const Json& args) -> Json {
        auto params = args.get<Params>();
        return Json(f(std::move(params)));
    };
    return Tool(std::move(name), std::move(description), std::move(schema), std::move(handler));
}

} // namespace forgeant

#endif // FORGEANT_TOOL_TOOL_HPP
