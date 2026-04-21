#include <agentforge/provider/openai.hpp>

#include <stdexcept>

namespace agentforge {

namespace {

void serialize_tool_result_messages(Json& messages, const Message& msg) {
    for (const auto& block : msg.content) {
        if (std::holds_alternative<ToolResultBlock>(block)) {
            const auto& result = std::get<ToolResultBlock>(block);
            Json tool_msg;
            tool_msg["role"] = "tool";
            tool_msg["tool_call_id"] = result.tool_use_id;
            tool_msg["content"] = result.content;
            messages.push_back(std::move(tool_msg));
        }
    }
}

Json serialize_tool(const ToolView& tool) {
    return Json::object({{"type", "function"},
                         {"function", Json::object({{"name", tool.name},
                                                    {"description", tool.description},
                                                    {"parameters", tool.parameters}})}});
}

Json serialize_message(const Message& msg) {
    Json msg_json;
    to_json(msg_json["role"], msg.role);

    std::string text_content;
    Json tool_calls = Json::array();

    for (const auto& block : msg.content) {
        if (std::holds_alternative<TextBlock>(block)) {
            text_content = std::get<TextBlock>(block).text;
        } else if (std::holds_alternative<ToolUseBlock>(block)) {
            const auto& tool = std::get<ToolUseBlock>(block);
            tool_calls.push_back(Json::object(
                {{"id", tool.id},
                 {"type", "function"},
                 {"function",
                  Json::object({{"name", tool.name}, {"arguments", tool.input.dump()}})}}));
        }
    }

    if (!tool_calls.empty()) {
        msg_json["tool_calls"] = std::move(tool_calls);
        msg_json["content"] = text_content.empty() ? Json(nullptr) : Json(text_content);
    } else {
        msg_json["content"] = text_content;
    }

    return msg_json;
}

} // namespace

OpenAiProvider::OpenAiProvider(HttpClient& client, ProviderConfig config)
    : client_(client), config_(std::move(config)) {
    if (config_.base_url.empty()) {
        config_.base_url = DEFAULT_BASE_URL;
    }
}

Json OpenAiProvider::serialize_request(const Conversation& conversation,
                                       const ChatRequest& request) const {
    Json body;
    body["model"] = config_.model;

    if (config_.max_tokens.has_value()) {
        body["max_tokens"] = config_.max_tokens.value();
    }

    if (config_.temperature.has_value()) {
        body["temperature"] = config_.temperature.value();
    }

    auto& messages = body["messages"] = Json::array();

    if (const auto& sp = conversation.system_prompt()) {
        messages.push_back(Json::object({{"role", "system"}, {"content", *sp}}));
    }

    for (const auto& msg : conversation.messages()) {
        if (msg.role == Role::tool) {
            serialize_tool_result_messages(messages, msg);
            continue;
        }
        messages.push_back(serialize_message(msg));
    }

    if (!request.tools.empty()) {
        body["tools"] = Json::array();
        for (const auto& tool : request.tools) {
            body["tools"].push_back(serialize_tool(tool));
        }
    }

    if (request.output_schema.has_value()) {
        auto schema_copy = *request.output_schema;
        if (schema_copy.value("type", "") == "object") {
            schema_copy["additionalProperties"] = false;
        }
        body["response_format"] =
            Json::object({{"type", "json_schema"},
                          {"json_schema", Json::object({{"name", "response"}, {"strict", true}})}});
        body["response_format"]["json_schema"]["schema"] = std::move(schema_copy);
    }

    return body;
}

LlmResponse OpenAiProvider::deserialize_response(const Json& json) {
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
            auto args = Json::object();
            if (func.contains("arguments") && func["arguments"].is_string()) {
                try {
                    args = Json::parse(func["arguments"].get<std::string>());
                } catch (...) {
                    args = Json::object({{"_raw", func["arguments"].get<std::string>()}});
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

LlmResponse OpenAiProvider::chat(const Conversation& conversation, const ChatRequest& request) {
    auto request_body = serialize_request(conversation, request);
    auto http_response = client_.post(endpoint_url(), auth_headers(), request_body.dump());

    if (http_response.status_code < 200 || http_response.status_code >= 300) {
        std::string error_msg =
            "OpenAI API error (HTTP " + std::to_string(http_response.status_code) + ")";
        if (!http_response.body.empty()) {
            try {
                auto err_json = Json::parse(http_response.body);
                if (err_json.contains("error") && err_json["error"].contains("message")) {
                    error_msg += ": " + err_json["error"]["message"].get<std::string>();
                }
            } catch (...) {
                error_msg += ": " + http_response.body;
            }
        }
        throw std::runtime_error(error_msg);
    }

    auto response_json = Json::parse(http_response.body);
    return deserialize_response(response_json);
}

} // namespace agentforge
