# Structured output {#structured-output-guide}

Sometimes you don't want a string back — you want a parsed struct. `Agent::run<T>()` attaches a JSON Schema derived from `T` to the model's request, parses the response, and returns an `forgeant::AgentResult<T>` with the parsed value in `output`.

## Define the target type

```cpp
struct MovieReview {
    std::string title;
    int rating;
    std::string summary;
};

template <>
struct forgeant::ParamSchema<MovieReview> {
    static forgeant::Json schema() {
        return forgeant::Schema::object()
            .property("title",   forgeant::Schema::string()
                                     .description("Movie title").build())
            .property("rating",  forgeant::Schema::integer()
                                     .description("Rating from 1 to 10").build())
            .property("summary", forgeant::Schema::string()
                                     .description("One sentence summary").build())
            .required({"title", "rating", "summary"})
            .build();
    }
};

void from_json(const forgeant::Json& j, MovieReview& out) {
    j.at("title").get_to(out.title);
    j.at("rating").get_to(out.rating);
    j.at("summary").get_to(out.summary);
}
```

## Call `run<T>()`

```cpp
auto result = agent->run<MovieReview>("Review the movie 'The Matrix' (1999).");

std::cout << result.output.title   << "\n";
std::cout << result.output.rating  << "/10\n";
std::cout << result.output.summary << "\n";
```

The `output` field is of type `MovieReview`, already parsed and populated.

## Retries on parse failure

If the model's response cannot be parsed into `T`, the agent appends a corrective user turn ("Your response did not match the required schema: …") and tries again. The retry budget comes from `forgeant::StructuredConfig::max_retries` (default: 2, giving 3 total attempts).

After the final attempt still fails, `forgeant::AgentRunError` is thrown with kind `structured_parse`. The exception carries the full conversation, so you can inspect what the model produced on each attempt:

```cpp
try {
    auto result = agent->run<MovieReview>("…");
} catch (const forgeant::AgentRunError& e) {
    if (e.kind() == forgeant::AgentRunError::Kind::structured_parse) {
        for (const auto& msg : e.conversation().messages()) {
            // inspect what went wrong
        }
    }
}
```

## Combining with tools

Tools work inside a typed run. The agent loop still calls tools for any `tool_use` blocks the model produces; the final turn — the one parsed into `T` — must carry the structured result. Use this when the model needs to gather information before producing a typed answer.

## Where to look next

- @ref structured — the full reference for `ParamSchema`, `Schema`, and `StructuredConfig`.
- @ref agents — for the `AgentResult<T>` shape and the error contract.

@example structured_output.cpp
