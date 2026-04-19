#include <agentforge/agent/agent.hpp>
#include <agentforge/http/curl_client.hpp>
#include <agentforge/provider/anthropic.hpp>
#include <agentforge/provider/openai.hpp>

#include <stdexcept>
#include <variant>

namespace agentforge {

Agent::Agent(LlmProvider& provider, AgentConfig config)
    : provider_(&provider), config_(std::move(config)) {}

Agent::Agent(std::unique_ptr<HttpClient> http, std::unique_ptr<LlmProvider> provider,
             AgentConfig config)
    : owned_http_(std::move(http)), owned_provider_(std::move(provider)),
      provider_(owned_provider_.get()), config_(std::move(config)) {}

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
        if (provider_config.base_url.empty()) {
            provider_config.base_url = "https://api.anthropic.com";
        }
        provider = std::make_unique<AnthropicProvider>(*http, std::move(provider_config));
    } else if (provider_name == "openai") {
        if (provider_config.base_url.empty()) {
            provider_config.base_url = "https://api.openai.com";
        }
        provider = std::make_unique<OpenAiProvider>(*http, std::move(provider_config));
    } else if (provider_name == "ollama") {
        if (provider_config.base_url.empty()) {
            provider_config.base_url = "http://localhost:11434";
        }
        provider = std::make_unique<OpenAiProvider>(*http, std::move(provider_config));
    } else {
        throw std::invalid_argument("unknown provider: " + provider_name);
    }

    AgentConfig agent_config{
        .max_iterations = options.max_iterations,
        .system_prompt = options.system_prompt,
    };

    return std::unique_ptr<Agent>(
        new Agent(std::move(http), std::move(provider), std::move(agent_config)));
}

void Agent::add_tool(Tool tool) {
    registry_.add(std::move(tool));
}

AgentResult Agent::run(const std::string& prompt) {
    Conversation conversation;
    if (!config_.system_prompt.empty()) {
        conversation.set_system_prompt(config_.system_prompt);
    }
    conversation.add(Message(Role::user, prompt));
    return execute_loop(conversation);
}

AgentResult Agent::chat(Conversation& conversation, const std::string& prompt) {
    conversation.add(Message(Role::user, prompt));
    return execute_loop(conversation);
}

AgentResult Agent::execute_loop(Conversation& conversation) {
    AgentResult result;
    auto tool_defs = registry_.definitions();

    for (int iteration = 0; iteration < config_.max_iterations; ++iteration) {
        LlmResponse response;
        if (tool_defs.empty()) {
            response = provider_->chat(conversation);
        } else {
            response = provider_->chat(conversation, std::span<const nlohmann::json>(tool_defs));
        }

        result.total_usage.input_tokens += response.usage.input_tokens;
        result.total_usage.output_tokens += response.usage.output_tokens;
        result.iterations = iteration + 1;
        result.finish_reason = response.finish_reason;
        result.message = response.message;

        conversation.add(response.message);

        bool has_tool_calls = false;
        std::vector<ContentBlock> tool_results;

        for (const auto& block : response.message.content) {
            if (!std::holds_alternative<ToolUseBlock>(block)) {
                continue;
            }
            has_tool_calls = true;
            const auto& tool_call = std::get<ToolUseBlock>(block);

            try {
                auto tool_output = registry_.execute(tool_call.name, tool_call.input);
                tool_results.emplace_back(ToolResultBlock{
                    .tool_use_id = tool_call.id,
                    .content = tool_output.dump(),
                });
            } catch (const std::exception& e) {
                tool_results.emplace_back(ToolResultBlock{
                    .tool_use_id = tool_call.id,
                    .content = e.what(),
                    .is_error = true,
                });
            }
        }

        if (!has_tool_calls) {
            for (const auto& block : response.message.content) {
                if (std::holds_alternative<TextBlock>(block)) {
                    result.text = std::get<TextBlock>(block).text;
                    break;
                }
            }
            return result;
        }

        conversation.add(Message(Role::tool, std::move(tool_results)));
    }

    result.finish_reason = "max_iterations";
    for (const auto& block : result.message.content) {
        if (std::holds_alternative<TextBlock>(block)) {
            result.text = std::get<TextBlock>(block).text;
            break;
        }
    }
    return result;
}

} // namespace agentforge
