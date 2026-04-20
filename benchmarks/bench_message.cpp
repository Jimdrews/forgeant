#include <agentforge/types.hpp>

#include <benchmark/benchmark.h>

static void BM_MessageCreate(benchmark::State& state) {
    for (auto _ : state) {
        agentforge::Message msg(agentforge::Role::user, "Hello, world!");
        benchmark::DoNotOptimize(msg);
    }
}
BENCHMARK(BM_MessageCreate);

static void BM_MessageToJson(benchmark::State& state) {
    agentforge::Message msg(agentforge::Role::assistant, "Hello, world!");
    for (auto _ : state) {
        agentforge::Json j = msg;
        benchmark::DoNotOptimize(j);
    }
}
BENCHMARK(BM_MessageToJson);

static void BM_MessageFromJson(benchmark::State& state) {
    agentforge::Message msg(agentforge::Role::assistant, "Hello, world!");
    agentforge::Json j = msg;
    for (auto _ : state) {
        auto restored = j.get<agentforge::Message>();
        benchmark::DoNotOptimize(restored);
    }
}
BENCHMARK(BM_MessageFromJson);
