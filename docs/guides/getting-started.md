# Getting started {#getting-started}

This guide takes you from a fresh checkout to your first `Agent::run()` call.

## Install

Forgeant builds with CMake and vcpkg. Add it as a dependency in your `vcpkg.json`, or clone the repository and build directly:

```bash
git clone https://github.com/Jimdrews/forgeant.git
cd forgeant
cmake --preset default
cmake --build build
```

See the repository [README](https://github.com/Jimdrews/forgeant) for platform-specific notes.

## Pick a provider

Forgeant ships with three built-in providers: Anthropic, OpenAI, and Ollama (which reuses the OpenAI wire format against a local model server).

| Provider     | `api_key`   | `model` example                      | Notes                                   |
|--------------|-------------|--------------------------------------|-----------------------------------------|
| `anthropic`  | required    | `claude-sonnet-4-20250514`           | Hits `api.anthropic.com` by default.    |
| `openai`     | required    | `gpt-4o-mini`                        | Hits `api.openai.com` by default.       |
| `ollama`     | empty       | `llama3`                             | Defaults `base_url` to `localhost:11434`. |

## First run

```cpp
#include <forgeant/forgeant.hpp>
#include <iostream>

int main() {
    auto agent = forgeant::Agent::create("anthropic", {
        .api_key = std::getenv("ANTHROPIC_API_KEY"),
        .model = "claude-sonnet-4-20250514",
        .system_prompt = "You are a helpful assistant. Be concise.",
    });

    auto result = agent->run("What is the capital of France?");

    std::cout << result.output << "\n";
    std::cout << "Used " << result.total_usage.input_tokens << " in, "
              << result.total_usage.output_tokens << " out\n";
}
```

`Agent::create()` builds an HTTP client and the matching provider from the options you pass. The returned `std::unique_ptr<Agent>` owns them; destroy it to release everything.

`run()` returns an `forgeant::AgentResult` containing the final text, the full `Conversation` that was assembled during the call, and usage counters. See @ref agents for the complete result shape.

## What to read next

- @ref tools — expose callable tools to the model.
- @ref structured — ask the model to return typed values instead of strings.
- @ref types — the message and conversation types you'll see in `AgentResult::conversation`.

@example basic_chat.cpp
