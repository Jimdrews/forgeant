#include <agentforge/tool/registry.hpp>

#include <benchmark/benchmark.h>

struct BenchParams {
    std::string text;
    int count = 0;
};

template <>
struct agentforge::ParamSchema<BenchParams> {
    static agentforge::Json schema() {
        return agentforge::Schema::object()
            .property("text", agentforge::Schema::string().build())
            .property("count", agentforge::Schema::integer().build())
            .build();
    }
};

inline void from_json(const agentforge::Json& j, BenchParams& p) {
    j.at("text").get_to(p.text);
    j.at("count").get_to(p.count);
}

static void BM_ToolDispatch(benchmark::State& state) {
    agentforge::ToolRegistry registry;
    registry.add(
        agentforge::make_tool<BenchParams>("echo", "Echo", [](BenchParams p) { return p.text; }));
    agentforge::Json args = agentforge::Json::object({{"text", "hello"}, {"count", 1}});

    for (auto _ : state) {
        auto result = registry.execute("echo", args);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ToolDispatch);

static void BM_SchemaGeneration(benchmark::State& state) {
    for (auto _ : state) {
        auto schema = agentforge::Schema::object()
                          .property("name", agentforge::Schema::string().build())
                          .property("age", agentforge::Schema::integer().build())
                          .required({"name"})
                          .build();
        benchmark::DoNotOptimize(schema);
    }
}
BENCHMARK(BM_SchemaGeneration);

static void BM_RegistryLookup(benchmark::State& state) {
    agentforge::ToolRegistry registry;
    for (int i = 0; i < 100; i++) {
        registry.add(agentforge::Tool(
            "tool_" + std::to_string(i), "desc", agentforge::Schema::object().build(),
            [](const agentforge::Json&) -> agentforge::Json { return nullptr; }));
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(registry.has("tool_50"));
    }
}
BENCHMARK(BM_RegistryLookup);
