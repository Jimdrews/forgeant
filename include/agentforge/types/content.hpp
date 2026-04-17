#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <variant>

namespace agentforge {

struct TextBlock {
    std::string text;

    bool operator==(const TextBlock&) const = default;
};

struct ToolUseBlock {
    std::string id;
    std::string name;
    nlohmann::json input;

    ToolUseBlock();
    ToolUseBlock(std::string id, std::string name, nlohmann::json input);

    ToolUseBlock(const ToolUseBlock&) = default;
    ToolUseBlock(ToolUseBlock&&) noexcept = default;
    ToolUseBlock& operator=(const ToolUseBlock&) = default;
    ToolUseBlock& operator=(ToolUseBlock&&) noexcept = default;
    ~ToolUseBlock() = default;

    bool operator==(const ToolUseBlock& other) const {
        return id == other.id && name == other.name && input == other.input;
    }
};

struct ToolResultBlock {
    std::string tool_use_id;
    std::string content;
    bool is_error = false;

    bool operator==(const ToolResultBlock&) const = default;
};

using ContentBlock = std::variant<TextBlock, ToolUseBlock, ToolResultBlock>;

void to_json(nlohmann::json& j, const TextBlock& block);
void from_json(const nlohmann::json& j, TextBlock& block);

void to_json(nlohmann::json& j, const ToolUseBlock& block);
void from_json(const nlohmann::json& j, ToolUseBlock& block);

void to_json(nlohmann::json& j, const ToolResultBlock& block);
void from_json(const nlohmann::json& j, ToolResultBlock& block);

void to_json(nlohmann::json& j, const ContentBlock& block);
void from_json(const nlohmann::json& j, ContentBlock& block);

} // namespace agentforge
