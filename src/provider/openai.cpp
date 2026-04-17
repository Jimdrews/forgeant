#include <agentforge/provider/openai.hpp>

#include <stdexcept>

namespace agentforge {

OpenAiProvider::OpenAiProvider(HttpClient& client, ProviderConfig config)
    : client_(client), config_(std::move(config)) {
    if (config_.base_url.empty()) {
        config_.base_url = DEFAULT_BASE_URL;
    }
}

nlohmann::json OpenAiProvider::serialize_request(const Conversation& conversation,
                                                 std::span<const nlohmann::json> tools) const {
    nlohmann::json request;
    request["model"] = config_.model;

    if (config_.max_tokens.has_value()) {
        request["max_tokens"] = config_.max_tokens.value();
    }

    if (config_.temperature.has_value()) {
        request["temperature"] = config_.temperature.value();
    }

    auto& messages = request["messages"] = nlohmann::json::array();

    if (const auto& sp = conversation.system_prompt()) {
        messages.push_back({{"role", "system"}, {"content", *sp}});
    }

    for (const auto& msg : conversation.messages()) {
        nlohmann::json msg_json;
        msg_json["role"] = msg.role;

        if (msg.role == Role::tool && msg.content.size() == 1 &&
            std::holds_alternative<ToolResultBlock>(msg.content[0])) {
            const auto& result = std::get<ToolResultBlock>(msg.content[0]);
            msg_json["role"] = "tool";
            msg_json["tool_call_id"] = result.tool_use_id;
            msg_json["content"] = result.content;
        } else if (msg.content.size() == 1 && std::holds_alternative<TextBlock>(msg.content[0])) {
            msg_json["content"] = std::get<TextBlock>(msg.content[0]).text;
        } else {
            auto& content = msg_json["content"] = nlohmann::json::array();
            for (const auto& block : msg.content) {
                nlohmann::json block_json;
                to_json(block_json, block);
                content.push_back(std::move(block_json));
            }
        }

        messages.push_back(std::move(msg_json));
    }

    if (!tools.empty()) {
        request["tools"] = nlohmann::json::array();
        for (const auto& tool : tools) {
            request["tools"].push_back(tool);
        }
    }

    return request;
}

LlmResponse OpenAiProvider::deserialize_response(const nlohmann::json& json) {
    LlmResponse response;
    response.model = json.value("model", "");

    if (json.contains("usage")) {
        const auto& usage = json["usage"];
        response.usage.input_tokens = usage.value("prompt_tokens", 0);
        response.usage.output_tokens = usage.value("completion_tokens", 0);
    }

    if (!json.contains("choices") || json["choices"].empty()) {
        throw std::runtime_error("OpenAI response missing choices");
    }

    const auto& choice = json["choices"][0];
    response.finish_reason = choice.value("finish_reason", "");

    const auto& msg = choice["message"];
    std::vector<ContentBlock> content_blocks;

    if (msg.contains("tool_calls")) {
        for (const auto& tc : msg["tool_calls"]) {
            const auto& func = tc["function"];
            auto args = nlohmann::json::object();
            if (func.contains("arguments") && func["arguments"].is_string()) {
                try {
                    args = nlohmann::json::parse(func["arguments"].get<std::string>());
                } catch (...) {
                    args = {{"_raw", func["arguments"].get<std::string>()}};
                }
            }
            content_blocks.emplace_back(
                ToolUseBlock(tc.value("id", ""), func.value("name", ""), std::move(args)));
        }
    }

    if (msg.contains("content") && !msg["content"].is_null()) {
        if (msg["content"].is_string()) {
            auto text = msg["content"].get<std::string>();
            if (!text.empty()) {
                content_blocks.emplace_back(TextBlock{.text = std::move(text)});
            }
        }
    }

    response.message = Message(Role::assistant, std::move(content_blocks));
    return response;
}

std::string OpenAiProvider::endpoint_url() const {
    return config_.base_url + "/v1/chat/completions";
}

HttpHeaders OpenAiProvider::auth_headers() const {
    return {{"Authorization", "Bearer " + config_.api_key}, {"content-type", "application/json"}};
}

LlmResponse OpenAiProvider::chat(const Conversation& conversation) {
    return chat(conversation, {});
}

LlmResponse OpenAiProvider::chat(const Conversation& conversation,
                                 std::span<const nlohmann::json> tools) {
    auto request_body = serialize_request(conversation, tools);
    auto http_response = client_.post(endpoint_url(), auth_headers(), request_body.dump());

    if (http_response.status_code < 200 || http_response.status_code >= 300) {
        std::string error_msg =
            "OpenAI API error (HTTP " + std::to_string(http_response.status_code) + ")";
        if (!http_response.body.empty()) {
            try {
                auto err_json = nlohmann::json::parse(http_response.body);
                if (err_json.contains("error") && err_json["error"].contains("message")) {
                    error_msg += ": " + err_json["error"]["message"].get<std::string>();
                }
            } catch (...) {
                error_msg += ": " + http_response.body;
            }
        }
        throw std::runtime_error(error_msg);
    }

    auto response_json = nlohmann::json::parse(http_response.body);
    return deserialize_response(response_json);
}

} // namespace agentforge
