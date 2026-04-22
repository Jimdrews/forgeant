#include <forgeant/provider/anthropic.hpp>
#include <stdexcept>

namespace forgeant {

namespace {

Json serialize_tool(const ToolView& tool) {
    return Json::object({{"name", tool.name},
                         {"description", tool.description},
                         {"input_schema", tool.parameters}});
}

} // namespace

AnthropicProvider::AnthropicProvider(HttpClient& client, ProviderConfig config)
    : client_(client), config_(std::move(config)) {
    if (config_.base_url.empty()) {
        config_.base_url = DEFAULT_BASE_URL;
    }
}

Json AnthropicProvider::serialize_request(const Conversation& conversation,
                                          const ChatRequest& request) const {
    Json body;
    body["model"] = config_.model;
    body["max_tokens"] = config_.max_tokens.value_or(1024);

    if (const auto& sp = conversation.system_prompt()) {
        body["system"] = *sp;
    }

    if (config_.temperature.has_value()) {
        body["temperature"] = config_.temperature.value();
    }

    auto& messages = body["messages"] = Json::array();
    for (const auto& msg : conversation.messages()) {
        Json msg_json;

        if (msg.role == Role::tool) {
            msg_json["role"] = "user";
        } else {
            to_json(msg_json["role"], msg.role);
        }

        auto& content = msg_json["content"] = Json::array();
        for (const auto& block : msg.content) {
            Json block_json;
            to_json(block_json, block);
            content.push_back(std::move(block_json));
        }

        messages.push_back(std::move(msg_json));
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
        body["output_config"]["format"]["type"] = "json_schema";
        body["output_config"]["format"]["schema"] = std::move(schema_copy);
    }

    return body;
}

LlmResponse AnthropicProvider::deserialize_response(const Json& json) {
    LlmResponse response;
    response.model = json.value("model", "");
    response.finish_reason = json.value("stop_reason", "");

    if (json.contains("usage")) {
        const auto& usage = json["usage"];
        response.usage.input_tokens = usage.value("input_tokens", 0);
        response.usage.output_tokens = usage.value("output_tokens", 0);
    }

    std::vector<ContentBlock> content_blocks;
    if (json.contains("content")) {
        for (const auto& block : json["content"]) {
            auto type = block.value("type", "");
            if (type == "text") {
                content_blocks.emplace_back(TextBlock{.text = block.value("text", "")});
            } else if (type == "tool_use") {
                content_blocks.emplace_back(ToolUseBlock(block.value("id", ""),
                                                         block.value("name", ""),
                                                         block.value("input", Json::object())));
            }
        }
    }

    response.message = Message(Role::assistant, std::move(content_blocks));
    return response;
}

std::string AnthropicProvider::endpoint_url() const {
    return config_.base_url + "/v1/messages";
}

HttpHeaders AnthropicProvider::auth_headers() const {
    return {{"x-api-key", config_.api_key},
            {"anthropic-version", API_VERSION},
            {"content-type", "application/json"}};
}

LlmResponse AnthropicProvider::chat(const Conversation& conversation, const ChatRequest& request) {
    auto request_body = serialize_request(conversation, request);
    auto http_response = client_.post(endpoint_url(), auth_headers(), request_body.dump());

    if (http_response.status_code < 200 || http_response.status_code >= 300) {
        std::string error_msg =
            "Anthropic API error (HTTP " + std::to_string(http_response.status_code) + ")";
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

} // namespace forgeant
