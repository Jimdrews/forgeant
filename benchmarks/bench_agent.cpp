#include <benchmark/benchmark.h>
#include <forgeant/agent/agent.hpp>
#include <forgeant/schema/schema.hpp>
#include <queue>

namespace {

class BenchProvider : public forgeant::LlmProvider {
  public:
    forgeant::LlmResponse canned;

    forgeant::LlmResponse chat(const forgeant::Conversation&,
                               const forgeant::ChatRequest&) override {
        return canned;
    }
};

} // namespace

static void BM_AgentRunNoTools(benchmark::State& state) {
    BenchProvider provider;
    provider.canned.message = forgeant::Message(forgeant::Role::assistant, "Hello");
    provider.canned.finish_reason = "end_turn";

    for (auto _ : state) {
        forgeant::Agent agent(provider);
        auto result = agent.run("Hi");
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AgentRunNoTools);
