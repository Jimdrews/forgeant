# Tools {#tools-guide}

Tools are how you give the model capabilities beyond text generation — calculators, database queries, HTTP calls, filesystem access. The agent's ReAct loop handles the back-and-forth: the model requests a tool, forgeant executes it, the result goes back into the conversation, and the model sees it on the next turn.

## Define a parameter struct

Start with a plain C++ struct describing the arguments your tool takes. Specialize `forgeant::ParamSchema` for it, and provide a `from_json` overload so the model's arguments can be parsed:

```cpp
struct WeatherParams {
    std::string city;
    std::string unit = "fahrenheit";
};

template <>
struct forgeant::ParamSchema<WeatherParams> {
    static forgeant::Json schema() {
        return forgeant::Schema::object()
            .property("city", forgeant::Schema::string()
                                  .description("The city name").build())
            .property("unit", forgeant::Schema::string()
                                  .description("Temperature unit")
                                  .enum_values({"fahrenheit", "celsius"})
                                  .default_value("fahrenheit")
                                  .build())
            .required({"city"})
            .build();
    }
};

void from_json(const forgeant::Json& j, WeatherParams& out) {
    j.at("city").get_to(out.city);
    if (j.contains("unit")) j.at("unit").get_to(out.unit);
}
```

## Build the tool with `make_tool`

```cpp
auto weather = forgeant::make_tool<WeatherParams>(
    "get_weather",
    "Get the current weather for a city",
    [](WeatherParams p) -> std::string {
        return "72°F and sunny in " + p.city;
    });

agent->add_tool(std::move(weather));
```

`make_tool` derives the JSON Schema from `ParamSchema<WeatherParams>`, wraps your lambda so it receives a deserialized `WeatherParams`, and turns the return value back into JSON for the model. You can return any type with a `to_json` overload (including `std::string`, primitives, or your own structs).

## Errors from tool handlers

Exceptions thrown inside the lambda propagate out of `Agent::run()` as an `forgeant::AgentRunError` with kind `provider_error`. If you want the model to see a failure and try something else instead, catch inside the lambda and return a JSON object describing the error:

```cpp
auto weather = forgeant::make_tool<WeatherParams>(
    "get_weather", "…",
    [](WeatherParams p) -> forgeant::Json {
        try {
            return fetch_weather(p.city);
        } catch (const std::exception& e) {
            return forgeant::Json::object({
                {"error", std::string(e.what())},
            });
        }
    });
```

## Where to look next

- @ref tools — the full reference for `Tool`, `make_tool`, and `ToolRegistry`.
- @ref structured — combine tools with typed output so the final answer is a parsed struct.
- @ref agents — see how the agent loop feeds tool results back to the model.

@example tool_usage.cpp
