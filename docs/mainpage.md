# Forgeant

Forgeant is a C++23 agent framework that provides tool use, structured output, and a ReAct-style agent loop across multiple LLM providers — all without leaving C++.

## Quick example

```cpp
#include <forgeant/forgeant.hpp>

auto agent = forgeant::Agent::create("anthropic", {
    .api_key = "sk-...",
    .model = "claude-sonnet-4-20250514",
});

auto result = agent->run("What is the capital of France?");
std::cout << result.output << std::endl;
```

## Guides

- @ref getting-started — install, pick a provider, first `Agent::run()`.
- @ref tools-guide — expose callable capabilities to the model.
- @ref structured-output-guide — `run<T>()` for typed responses.
- @ref multi-turn-guide — carrying a `Conversation` across calls.

## Reference

- [Modules](modules.html) — the six semantic groups: agents, tools, providers, structured output, core types, transport.
- [All classes](annotated.html) — full public surface.
- [Namespaces](namespaces.html) — browse by namespace.
- [Files](files.html) — browse the headers under `include/forgeant/`.
- [Examples](examples.html) — complete runnable programs.

## Source and issues

Forgeant is open source under the MIT license. Development happens at [github.com/Jimdrews/forgeant](https://github.com/Jimdrews/forgeant).
