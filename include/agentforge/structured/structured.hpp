#ifndef AGENTFORGE_STRUCTURED_STRUCTURED_HPP
#define AGENTFORGE_STRUCTURED_STRUCTURED_HPP

#include <agentforge/agent/error.hpp>
#include <agentforge/agent/result.hpp>
#include <agentforge/provider/chat_request.hpp>
#include <agentforge/provider/provider.hpp>
#include <agentforge/schema/param_schema.hpp>
#include <agentforge/structured/config.hpp>
#include <agentforge/types/conversation.hpp>

#include <exception>
#include <string>
#include <utility>

namespace agentforge {

nlohmann::json extract_json_from_response(const LlmResponse& response);

template <typename T>
AgentResult<T> structured(LlmProvider& provider, Conversation working,
                          StructuredConfig config = {}) {
    auto schema = ParamSchema<T>::schema();
    Usage accumulated;
    std::string last_finish_reason;
    std::string last_error;

    for (int attempt = 0; attempt <= config.max_retries; ++attempt) {
        ChatRequest request;
        request.output_schema = schema;

        LlmResponse response;
        try {
            response = provider.chat(working, request);
        } catch (const std::exception& e) {
            throw AgentRunError(AgentRunError::Kind::provider_error, e.what(), std::move(working),
                                accumulated, attempt + 1, "error");
        }

        accumulated.input_tokens += response.usage.input_tokens;
        accumulated.output_tokens += response.usage.output_tokens;
        last_finish_reason = response.finish_reason;
        working.add(response.message);

        try {
            auto json = extract_json_from_response(response);
            AgentResult<T> result;
            result.output = json.template get<T>();
            result.conversation = std::move(working);
            result.total_usage = accumulated;
            result.iterations = attempt + 1;
            result.finish_reason = std::move(last_finish_reason);
            return result;
        } catch (const std::exception& e) {
            last_error = e.what();
            if (attempt < config.max_retries) {
                working.add(Message(
                    Role::user, "Your response did not match the required schema: " + last_error +
                                    ". Please respond with valid JSON matching the schema."));
            }
        }
    }

    throw AgentRunError(AgentRunError::Kind::structured_parse,
                        "structured output failed after " + std::to_string(config.max_retries + 1) +
                            " attempts: " + last_error,
                        std::move(working), accumulated, config.max_retries + 1,
                        std::move(last_finish_reason));
}

} // namespace agentforge

#endif // AGENTFORGE_STRUCTURED_STRUCTURED_HPP
