#include <agentforge/types/content.hpp>

#include <stdexcept>

namespace agentforge {

ToolUseBlock::ToolUseBlock() = default;

ToolUseBlock::ToolUseBlock(std::string id, std::string name, nlohmann::json input)
    : id(std::move(id)), name(std::move(name)), input(std::move(input)) {}

void to_json(nlohmann::json& j, const TextBlock& block) {
    j = {{"type", "text"}, {"text", block.text}};
}

void from_json(const nlohmann::json& j, TextBlock& block) {
    j.at("text").get_to(block.text);
}

void to_json(nlohmann::json& j, const ToolUseBlock& block) {
    j = {{"type", "tool_use"}, {"id", block.id}, {"name", block.name}, {"input", block.input}};
}

void from_json(const nlohmann::json& j, ToolUseBlock& block) {
    j.at("id").get_to(block.id);
    j.at("name").get_to(block.name);
    j.at("input").get_to(block.input);
}

void to_json(nlohmann::json& j, const ToolResultBlock& block) {
    j = {{"type", "tool_result"},
         {"tool_use_id", block.tool_use_id},
         {"content", block.content},
         {"is_error", block.is_error}};
}

void from_json(const nlohmann::json& j, ToolResultBlock& block) {
    j.at("tool_use_id").get_to(block.tool_use_id);
    j.at("content").get_to(block.content);
    block.is_error = j.value("is_error", false);
}

void to_json(nlohmann::json& j, const ContentBlock& block) {
    std::visit([&j](const auto& b) { to_json(j, b); }, block);
}

void from_json(const nlohmann::json& j, ContentBlock& block) {
    auto type = j.at("type").get<std::string>();
    if (type == "text") {
        TextBlock b;
        from_json(j, b);
        block = b;
    } else if (type == "tool_use") {
        ToolUseBlock b;
        from_json(j, b);
        block = b;
    } else if (type == "tool_result") {
        ToolResultBlock b;
        from_json(j, b);
        block = b;
    } else {
        throw std::invalid_argument("unknown content block type: " + type);
    }
}

} // namespace agentforge
