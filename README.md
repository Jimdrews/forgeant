<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="docs/logo-dark.svg">
    <img src="docs/logo-light.svg" alt="forgeant" width="400">
  </picture>
</p>

<p align="center">
  <strong>Build LLM-powered agents in C++.</strong>
</p>

<p align="center">
  <a href="https://github.com/Jimdrews/forgeant/actions/workflows/ci.yml"><img src="https://github.com/Jimdrews/forgeant/actions/workflows/ci.yml/badge.svg" alt="CI"></a>
  <a href="LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="License: MIT"></a>
  <img src="https://img.shields.io/badge/C%2B%2B-23-blue.svg" alt="C++23">
  <img src="https://img.shields.io/badge/version-0.2.0-green.svg" alt="Version">
</p>

---

Forgeant is a C++23 agent framework that gives you tool use, structured output, and a ReAct agent loop across multiple LLM providers -- without leaving C++.

```cpp
#include <forgeant/forgeant.hpp>

auto agent = forgeant::Agent::create("anthropic", {
    .api_key = "sk-...",
    .model = "claude-sonnet-4-20250514",
});

auto result = agent->run("What is the capital of France?");
std::cout << result.output << std::endl;
```

## Why Forgeant?

- **Native C++ agents.** No Python subprocess, no FFI bridge, no runtime overhead. Your agent runs where your application runs.
- **Type-safe tools.** Define tools as C++ structs. JSON Schema is generated from your types. The agent calls them automatically.
- **Structured output.** Get typed C++ structs back from LLMs. Not strings. Not JSON you have to parse yourself.
- **Multi-provider.** Anthropic, OpenAI, and Ollama from the same API. Switch with one parameter.
- **Tested seriously.** ASan, UBSan, Valgrind, clang-tidy, 139 tests across GCC, Clang, and macOS on every commit.

## Get Started

```cmake
include(FetchContent)
FetchContent_Declare(forgeant
    GIT_REPOSITORY https://github.com/Jimdrews/forgeant.git
    GIT_TAG main)
FetchContent_MakeAvailable(forgeant)
target_link_libraries(your_target PRIVATE forgeant::forgeant)
```

```bash
# Or build from source
git clone https://github.com/Jimdrews/forgeant.git && cd forgeant
cmake --preset dev && cmake --build --preset dev && ctest --preset dev
```

## What You Can Build

**Give your agent tools:**

```cpp
agent->add_tool(forgeant::make_tool<WeatherParams>(
    "get_weather", "Get current weather",
    [](WeatherParams p) { return "72F and sunny in " + p.city; }
));

auto result = agent->run("What's the weather in Denver?");
```

**Get structured data back:**

```cpp
auto result = agent->run<MovieReview>("Review 'The Matrix' (1999).");
std::cout << result.output.title << ": " << result.output.rating << "/10" << std::endl;
```

**Run multi-step reasoning:**

```cpp
auto agent = forgeant::Agent::create("anthropic", {
    .api_key = "sk-...",
    .model = "claude-sonnet-4-20250514",
    .max_iterations = 10,
});

agent->add_tool(search_tool);
agent->add_tool(calculator_tool);

auto result = agent->run("Find the average price of items matching 'widget'.");
// Agent reasons, calls tools, and responds — automatically
```

## Supported Compilers

GCC 14+ &bull; Clang 19+ &bull; AppleClang (latest) &bull; C++23 required

## Contributing

Contributions are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## License

[MIT](LICENSE)
