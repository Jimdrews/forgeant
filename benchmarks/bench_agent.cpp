#include <agentforge/agent/agent.hpp>
#include <agentforge/schema/schema.hpp>

#include <benchmark/benchmark.h>
#include <queue>

namespace {

class BenchProvider : public agentforge::LlmProvider {
  public:
    agentforge::LlmResponse canned;

    agentforge::LlmResponse chat(const agentforge::Conversation&,
                                 const agentforge::ChatRequest&) override {
        return canned;
    }
};

} // namespace

static void BM_AgentRunNoTools(benchmark::State& state) {
    BenchProvider provider;
    provider.canned.message = agentforge::Message(agentforge::Role::assistant, "Hello");
    provider.canned.finish_reason = "end_turn";

    for (auto _ : state) {
        agentforge::Agent agent(provider);
        auto result = agent.run("Hi");
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_AgentRunNoTools);
