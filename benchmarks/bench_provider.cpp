#include <benchmark/benchmark.h>
#include <forgeant/provider/anthropic.hpp>
#include <forgeant/provider/openai.hpp>

#include "../tests/mock_http_client.hpp"

namespace {

const std::string ANTHROPIC_RESPONSE = R"({
    "content": [{"type": "text", "text": "Hello!"}],
    "model": "claude-sonnet-4-20250514",
    "stop_reason": "end_turn",
    "usage": {"input_tokens": 10, "output_tokens": 5}
})";

const std::string OPENAI_RESPONSE = R"({
    "choices": [{"message": {"role": "assistant", "content": "Hello!"}, "finish_reason": "stop"}],
    "model": "gpt-4o",
    "usage": {"prompt_tokens": 10, "completion_tokens": 5}
})";

} // namespace

static void BM_AnthropicSerializeDeserialize(benchmark::State& state) {
    forgeant::testing::MockHttpClient mock;
    mock.canned_response.status_code = 200;
    mock.canned_response.body = ANTHROPIC_RESPONSE;

    forgeant::ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    forgeant::AnthropicProvider provider(mock, config);

    forgeant::Conversation conv("You are helpful.");
    conv.add(forgeant::Message(forgeant::Role::user, "Hello world"));

    for (auto _ : state) {
        auto response = provider.chat(conv);
        benchmark::DoNotOptimize(response);
    }
}
BENCHMARK(BM_AnthropicSerializeDeserialize);

static void BM_OpenAiSerializeDeserialize(benchmark::State& state) {
    forgeant::testing::MockHttpClient mock;
    mock.canned_response.status_code = 200;
    mock.canned_response.body = OPENAI_RESPONSE;

    forgeant::ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    forgeant::OpenAiProvider provider(mock, config);

    forgeant::Conversation conv("You are helpful.");
    conv.add(forgeant::Message(forgeant::Role::user, "Hello world"));

    for (auto _ : state) {
        auto response = provider.chat(conv);
        benchmark::DoNotOptimize(response);
    }
}
BENCHMARK(BM_OpenAiSerializeDeserialize);
