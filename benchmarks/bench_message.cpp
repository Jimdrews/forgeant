#include <benchmark/benchmark.h>
#include <forgeant/types.hpp>

static void BM_MessageCreate(benchmark::State& state) {
    for (auto _ : state) {
        forgeant::Message msg(forgeant::Role::user, "Hello, world!");
        benchmark::DoNotOptimize(msg);
    }
}
BENCHMARK(BM_MessageCreate);

static void BM_MessageToJson(benchmark::State& state) {
    forgeant::Message msg(forgeant::Role::assistant, "Hello, world!");
    for (auto _ : state) {
        forgeant::Json j;
        to_json(j, msg);
        benchmark::DoNotOptimize(j);
    }
}
BENCHMARK(BM_MessageToJson);

static void BM_MessageFromJson(benchmark::State& state) {
    forgeant::Message msg(forgeant::Role::assistant, "Hello, world!");
    forgeant::Json j;
    to_json(j, msg);
    for (auto _ : state) {
        auto restored = j.get<forgeant::Message>();
        benchmark::DoNotOptimize(restored);
    }
}
BENCHMARK(BM_MessageFromJson);
