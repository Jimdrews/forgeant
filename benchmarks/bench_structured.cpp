#include <benchmark/benchmark.h>
#include <forgeant/provider/anthropic.hpp>
#include <forgeant/structured/structured.hpp>

#include "../tests/mock_http_client.hpp"

struct BenchOutput {
    std::string city;
    double temperature = 0.0;
    std::string unit;
};

template <>
struct forgeant::ParamSchema<BenchOutput> {
    static forgeant::Json schema() {
        return forgeant::Schema::object()
            .property("city", forgeant::Schema::string().build())
            .property("temperature", forgeant::Schema::number().build())
            .property("unit", forgeant::Schema::string().build())
            .required({"city", "temperature", "unit"})
            .build();
    }
};

inline void from_json(const forgeant::Json& j, BenchOutput& out) {
    j.at("city").get_to(out.city);
    j.at("temperature").get_to(out.temperature);
    j.at("unit").get_to(out.unit);
}

static void BM_StructuredDeserialization(benchmark::State& state) {
    forgeant::testing::MockHttpClient mock;
    forgeant::Json response = forgeant::Json::object(
        {{"content",
          forgeant::Json::array({forgeant::Json::object(
              {{"type", "text"},
               {"text", R"({"city":"Denver","temperature":72.5,"unit":"fahrenheit"})"}})})},
         {"model", "claude-sonnet-4-20250514"},
         {"stop_reason", "end_turn"},
         {"usage", forgeant::Json::object({{"input_tokens", 10}, {"output_tokens", 5}})}});
    mock.canned_response.status_code = 200;
    mock.canned_response.body = response.dump();

    forgeant::ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    forgeant::AnthropicProvider provider(mock, config);

    for (auto _ : state) {
        forgeant::Conversation conv;
        conv.add(forgeant::Message(forgeant::Role::user, "weather"));
        auto result = forgeant::structured<BenchOutput>(provider, conv, {.max_retries = 0});
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_StructuredDeserialization);
