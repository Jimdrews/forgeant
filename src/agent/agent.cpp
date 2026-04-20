#include <agentforge/agent/agent.hpp>
#include <agentforge/http/curl_client.hpp>
#include <agentforge/provider/anthropic.hpp>
#include <agentforge/provider/openai.hpp>

#include <algorithm>
#include <ranges>
#include <stdexcept>
#include <variant>

namespace agentforge {

namespace {

std::vector<ContentBlock> dispatch_tools(ToolRegistry& registry, const Message& message) {
    std::vector<ContentBlock> results;
    for (const auto& block : message.content) {
        if (!std::holds_alternative<ToolUseBlock>(block)) {
            continue;
        }
        const auto& tool_call = std::get<ToolUseBlock>(block);
        try {
            auto output = registry.execute(tool_call.name, tool_call.input);
            results.emplace_back(ToolResultBlock{
                .tool_use_id = tool_call.id,
                .content = output.dump(),
            });
        } catch (const std::exception& e) {
            results.emplace_back(ToolResultBlock{
                .tool_use_id = tool_call.id,
                .content = e.what(),
                .is_error = true,
            });
        }
    }
    return results;
}

bool has_tool_use(const Message& message) {
    return std::ranges::any_of(message.content, [](const ContentBlock& block) {
        return std::holds_alternative<ToolUseBlock>(block);
    });
}

} // namespace

Agent::Agent(LlmProvider& provider, AgentOptions options)
    : provider_(&provider), options_(std::move(options)) {}

Agent::Agent(std::unique_ptr<HttpClient> http, std::unique_ptr<LlmProvider> provider,
             AgentOptions options)
    : owned_http_(std::move(http)), owned_provider_(std::move(provider)),
      provider_(owned_provider_.get()), options_(std::move(options)) {}

std::unique_ptr<Agent> Agent::create(const std::string& provider_name,
                                     const AgentOptions& options) {
    auto http = std::make_unique<CurlHttpClient>();

    ProviderConfig provider_config{
        .api_key = options.api_key,
        .model = options.model,
        .base_url = options.base_url,
        .max_tokens = options.max_tokens,
        .temperature = options.temperature,
    };

    std::unique_ptr<LlmProvider> provider;

    if (provider_name == "anthropic") {
        provider = std::make_unique<AnthropicProvider>(*http, std::move(provider_config));
    } else if (provider_name == "openai") {
        provider = std::make_unique<OpenAiProvider>(*http, std::move(provider_config));
    } else if (provider_name == "ollama") {
        if (provider_config.base_url.empty()) {
            provider_config.base_url = "http://localhost:11434";
        }
        provider = std::make_unique<OpenAiProvider>(*http, std::move(provider_config));
    } else {
        throw std::invalid_argument("unknown provider: " + provider_name);
    }

    return std::unique_ptr<Agent>(new Agent(std::move(http), std::move(provider), options));
}

void Agent::add_tool(Tool tool) {
    registry_.add(std::move(tool));
}

void Agent::apply_system_prompt(Conversation& working, const RunOverrides& overrides) const {
    if (working.system_prompt().has_value()) {
        return;
    }
    if (overrides.system_prompt.has_value()) {
        working.set_system_prompt(*overrides.system_prompt);
        return;
    }
    if (!options_.system_prompt.empty()) {
        working.set_system_prompt(options_.system_prompt);
    }
}

std::string Agent::extract_last_text(const Conversation& conv) {
    for (const auto& msg : std::ranges::reverse_view(conv.messages())) {
        if (msg.role != Role::assistant) {
            continue;
        }
        for (const auto& block : msg.content) {
            if (std::holds_alternative<TextBlock>(block)) {
                return std::get<TextBlock>(block).text;
            }
        }
    }
    return {};
}

Agent::LoopResult Agent::execute_loop(Conversation working, const RunOverrides& overrides,
                                      const std::optional<Json>& output_schema) {
    auto tool_views = registry_.tools();
    int max_iterations = overrides.max_iterations.value_or(options_.max_iterations);

    LoopResult result;
    Usage accumulated;

    for (int iteration = 0; iteration < max_iterations; ++iteration) {
        ChatRequest request;
        request.tools = std::span<const ToolView>(tool_views);
        request.output_schema = output_schema;

        LlmResponse response;
        try {
            response = provider_->chat(working, request);
        } catch (const std::exception& e) {
            throw AgentRunError(AgentRunError::Kind::provider_error, e.what(), std::move(working),
                                accumulated, iteration + 1, "error");
        }

        accumulated.input_tokens += response.usage.input_tokens;
        accumulated.output_tokens += response.usage.output_tokens;

        working.add(response.message);

        if (!has_tool_use(response.message)) {
            result.conversation = std::move(working);
            result.total_usage = accumulated;
            result.iterations = iteration + 1;
            result.finish_reason = response.finish_reason;
            return result;
        }

        auto tool_results = dispatch_tools(registry_, response.message);
        working.add(Message(Role::tool, std::move(tool_results)));
    }

    throw AgentRunError(AgentRunError::Kind::max_iterations,
                        "max_iterations reached without a terminal response", std::move(working),
                        accumulated, max_iterations, "max_iterations");
}

} // namespace agentforge
