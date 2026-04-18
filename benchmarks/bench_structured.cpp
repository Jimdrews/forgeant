#include <agentforge/provider/anthropic.hpp>
#include <agentforge/structured/structured.hpp>

#include <benchmark/benchmark.h>

#include "../tests/mock_http_client.hpp"

struct BenchOutput {
    std::string city;
    double temperature = 0.0;
    std::string unit;
};

template <>
struct agentforge::ParamSchema<BenchOutput> {
    static nlohmann::json schema() {
        return agentforge::Schema::object()
            .property("city", agentforge::Schema::string().build())
            .property("temperature", agentforge::Schema::number().build())
            .property("unit", agentforge::Schema::string().build())
            .required({"city", "temperature", "unit"})
            .build();
    }
};

inline void from_json(const nlohmann::json& j, BenchOutput& out) {
    j.at("city").get_to(out.city);
    j.at("temperature").get_to(out.temperature);
    j.at("unit").get_to(out.unit);
}

static void BM_StructuredDeserialization(benchmark::State& state) {
    agentforge::testing::MockHttpClient mock;
    nlohmann::json response = {
        {"content",
         {{{"type", "text"},
           {"text", R"({"city":"Denver","temperature":72.5,"unit":"fahrenheit"})"}}}},
        {"model", "claude-sonnet-4-20250514"},
        {"stop_reason", "end_turn"},
        {"usage", {{"input_tokens", 10}, {"output_tokens", 5}}}};
    mock.canned_response.status_code = 200;
    mock.canned_response.body = response.dump();

    agentforge::ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    agentforge::AnthropicProvider provider(mock, config);

    for (auto _ : state) {
        agentforge::Conversation conv;
        conv.add(agentforge::Message(agentforge::Role::user, "weather"));
        auto result = agentforge::structured<BenchOutput>(provider, conv, {.max_retries = 0});
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StructuredDeserialization);
