#include <agentforge/provider/anthropic.hpp>
#include <agentforge/provider/openai.hpp>

#include <benchmark/benchmark.h>

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
    agentforge::testing::MockHttpClient mock;
    mock.canned_response.status_code = 200;
    mock.canned_response.body = ANTHROPIC_RESPONSE;

    agentforge::ProviderConfig config{.api_key = "key", .model = "claude-sonnet-4-20250514"};
    agentforge::AnthropicProvider provider(mock, config);

    agentforge::Conversation conv("You are helpful.");
    conv.add(agentforge::Message(agentforge::Role::user, "Hello world"));

    for (auto _ : state) {
        auto response = provider.chat(conv);
        benchmark::DoNotOptimize(response);
    }
}
BENCHMARK(BM_AnthropicSerializeDeserialize);

static void BM_OpenAiSerializeDeserialize(benchmark::State& state) {
    agentforge::testing::MockHttpClient mock;
    mock.canned_response.status_code = 200;
    mock.canned_response.body = OPENAI_RESPONSE;

    agentforge::ProviderConfig config{.api_key = "key", .model = "gpt-4o"};
    agentforge::OpenAiProvider provider(mock, config);

    agentforge::Conversation conv("You are helpful.");
    conv.add(agentforge::Message(agentforge::Role::user, "Hello world"));

    for (auto _ : state) {
        auto response = provider.chat(conv);
        benchmark::DoNotOptimize(response);
    }
}
BENCHMARK(BM_OpenAiSerializeDeserialize);
