#ifndef AGENTFORGE_STRUCTURED_STRUCTURED_HPP
#define AGENTFORGE_STRUCTURED_STRUCTURED_HPP

#include <agentforge/provider/provider.hpp>
#include <agentforge/schema/param_schema.hpp>
#include <agentforge/structured/config.hpp>
#include <agentforge/types/conversation.hpp>

#include <stdexcept>
#include <string>

namespace agentforge {

nlohmann::json extract_json_from_response(const LlmResponse& response);

template <typename T>
T structured(LlmProvider& provider, Conversation conversation, StructuredConfig config = {}) {
    auto schema = ParamSchema<T>::schema();
    std::string last_error;

    for (int attempt = 0; attempt <= config.max_retries; ++attempt) {
        auto response = provider.chat(conversation, schema);

        try {
            auto json = extract_json_from_response(response);
            return json.template get<T>();
        } catch (const std::exception& e) {
            last_error = e.what();
            if (attempt < config.max_retries) {
                conversation.add(Message(Role::assistant,
                                         std::get<TextBlock>(response.message.content[0]).text));
                conversation.add(Message(
                    Role::user, "Your response did not match the required schema: " + last_error +
                                    ". Please respond with valid JSON matching the schema."));
            }
        }
    }

    throw std::runtime_error("structured output failed after " +
                             std::to_string(config.max_retries + 1) + " attempts: " + last_error);
}

} // namespace agentforge

#endif // AGENTFORGE_STRUCTURED_STRUCTURED_HPP
