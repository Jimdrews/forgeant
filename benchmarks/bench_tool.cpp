#include <benchmark/benchmark.h>
#include <forgeant/tool/registry.hpp>

struct BenchParams {
    std::string text;
    int count = 0;
};

template <>
struct forgeant::ParamSchema<BenchParams> {
    static forgeant::Json schema() {
        return forgeant::Schema::object()
            .property("text", forgeant::Schema::string().build())
            .property("count", forgeant::Schema::integer().build())
            .build();
    }
};

inline void from_json(const forgeant::Json& j, BenchParams& p) {
    j.at("text").get_to(p.text);
    j.at("count").get_to(p.count);
}

static void BM_ToolDispatch(benchmark::State& state) {
    forgeant::ToolRegistry registry;
    registry.add(
        forgeant::make_tool<BenchParams>("echo", "Echo", [](BenchParams p) { return p.text; }));
    forgeant::Json args = forgeant::Json::object({{"text", "hello"}, {"count", 1}});

    for (auto _ : state) {
        auto result = registry.execute("echo", args);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_ToolDispatch);

static void BM_SchemaGeneration(benchmark::State& state) {
    for (auto _ : state) {
        auto schema = forgeant::Schema::object()
                          .property("name", forgeant::Schema::string().build())
                          .property("age", forgeant::Schema::integer().build())
                          .required({"name"})
                          .build();
        benchmark::DoNotOptimize(schema);
    }
}
BENCHMARK(BM_SchemaGeneration);

static void BM_RegistryLookup(benchmark::State& state) {
    forgeant::ToolRegistry registry;
    for (int i = 0; i < 100; i++) {
        registry.add(
            forgeant::Tool("tool_" + std::to_string(i), "desc", forgeant::Schema::object().build(),
                           [](const forgeant::Json&) -> forgeant::Json { return nullptr; }));
    }

    for (auto _ : state) {
        benchmark::DoNotOptimize(registry.has("tool_50"));
    }
}
BENCHMARK(BM_RegistryLookup);
