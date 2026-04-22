#include <benchmark/benchmark.h>
#include <forgeant/version.hpp>

static void BM_VersionString(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(forgeant::version_string());
    }
}

BENCHMARK(BM_VersionString);
