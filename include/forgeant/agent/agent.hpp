#ifndef FORGEANT_AGENT_AGENT_HPP
#define FORGEANT_AGENT_AGENT_HPP

#include <exception>
#include <forgeant/agent/error.hpp>
#include <forgeant/agent/options.hpp>
#include <forgeant/agent/result.hpp>
#include <forgeant/agent/run_overrides.hpp>
#include <forgeant/http/client.hpp>
#include <forgeant/json/json.hpp>
#include <forgeant/provider/chat_request.hpp>
#include <forgeant/provider/provider.hpp>
#include <forgeant/schema/param_schema.hpp>
#include <forgeant/structured/config.hpp>
#include <forgeant/tool/registry.hpp>
#include <forgeant/types/conversation.hpp>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

namespace forgeant {

class Agent {
  public:
    static std::unique_ptr<Agent> create(const std::string& provider, const AgentOptions& options);

    Agent(LlmProvider& provider, AgentOptions options = {});

    void add_tool(Tool tool);

    template <typename T = std::string>
    AgentResult<T> run(std::string_view prompt, RunOverrides overrides = {}) {
        Conversation working;
        apply_system_prompt(working, overrides);
        working.add(Message(Role::user, std::string(prompt)));
        return execute<T>(std::move(working), overrides);
    }

    template <typename T = std::string>
    AgentResult<T> run(const Conversation& conversation, RunOverrides overrides = {}) {
        Conversation working = conversation;
        apply_system_prompt(working, overrides);
        return execute<T>(std::move(working), overrides);
    }

  private:
    std::unique_ptr<HttpClient> owned_http_;
    std::unique_ptr<LlmProvider> owned_provider_;
    LlmProvider* provider_;
    ToolRegistry registry_;
    AgentOptions options_;

    Agent(std::unique_ptr<HttpClient> http, std::unique_ptr<LlmProvider> provider,
          AgentOptions options);

    struct LoopResult {
        Conversation conversation;
        Usage total_usage;
        int iterations = 0;
        std::string finish_reason;
    };

    LoopResult execute_loop(Conversation working, const RunOverrides& overrides,
                            const std::optional<Json>& output_schema);

    void apply_system_prompt(Conversation& working, const RunOverrides& overrides) const;

    static std::string extract_last_text(const Conversation& conv);

    template <typename T>
    AgentResult<T> execute(Conversation working, const RunOverrides& overrides) {
        if constexpr (std::is_same_v<T, std::string>) {
            auto loop = execute_loop(std::move(working), overrides, std::nullopt);
            AgentResult<std::string> result;
            result.output = extract_last_text(loop.conversation);
            result.total_usage = loop.total_usage;
            result.iterations = loop.iterations;
            result.finish_reason = std::move(loop.finish_reason);
            result.conversation = std::move(loop.conversation);
            return result;
        } else {
            auto schema = ParamSchema<T>::schema();
            StructuredConfig cfg;
            Usage accumulated;
            int total_iterations = 0;
            std::string last_finish_reason;
            std::string last_error;

            for (int attempt = 0; attempt <= cfg.max_retries; ++attempt) {
                auto loop = execute_loop(std::move(working), overrides, schema);
                working = std::move(loop.conversation);
                accumulated.input_tokens += loop.total_usage.input_tokens;
                accumulated.output_tokens += loop.total_usage.output_tokens;
                total_iterations += loop.iterations;
                last_finish_reason = std::move(loop.finish_reason);

                auto text = extract_last_text(working);
                try {
                    auto json = Json::parse(text);
                    AgentResult<T> result;
                    result.output = json.template get<T>();
                    result.conversation = std::move(working);
                    result.total_usage = accumulated;
                    result.iterations = total_iterations;
                    result.finish_reason = std::move(last_finish_reason);
                    return result;
                } catch (const std::exception& e) {
                    last_error = e.what();
                    if (attempt < cfg.max_retries) {
                        working.add(Message(
                            Role::user,
                            "Your response did not match the required schema: " + last_error +
                                ". Please respond with valid JSON matching the schema."));
                    }
                }
            }

            throw AgentRunError(
                AgentRunError::Kind::structured_parse,
                "structured output failed after " + std::to_string(cfg.max_retries + 1) +
                    " attempts: " + last_error,
                std::move(working), accumulated, total_iterations, std::move(last_finish_reason));
        }
    }
};

} // namespace forgeant

#endif // FORGEANT_AGENT_AGENT_HPP
