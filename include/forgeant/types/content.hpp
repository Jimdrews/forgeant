#ifndef FORGEANT_TYPES_CONTENT_HPP
#define FORGEANT_TYPES_CONTENT_HPP

#include <forgeant/json/json.hpp>
#include <string>
#include <variant>

namespace forgeant {

/**
 * @ingroup types
 * @brief Plain-text content inside a message.
 */
struct TextBlock {
    std::string text;

    bool operator==(const TextBlock&) const = default;
};

/**
 * @ingroup types
 * @brief A model-issued tool call: name, unique id, and JSON arguments.
 */
struct ToolUseBlock {
    std::string id;
    std::string name;
    Json input;

    ToolUseBlock();
    ToolUseBlock(std::string id, std::string name, Json input);

    ToolUseBlock(const ToolUseBlock&) = default;
    ToolUseBlock(ToolUseBlock&&) noexcept = default;
    ToolUseBlock& operator=(const ToolUseBlock&) = default;
    ToolUseBlock& operator=(ToolUseBlock&&) noexcept = default;
    ~ToolUseBlock() = default;

    bool operator==(const ToolUseBlock& other) const {
        return id == other.id && name == other.name && input == other.input;
    }
};

/**
 * @ingroup types
 * @brief The result of executing a tool call, referenced back to its originating `ToolUseBlock`.
 */
struct ToolResultBlock {
    std::string tool_use_id;
    std::string content;
    bool is_error = false;

    bool operator==(const ToolResultBlock&) const = default;
};

/**
 * @ingroup types
 * @brief One of `TextBlock`, `ToolUseBlock`, or `ToolResultBlock` — the atoms of a message.
 */
using ContentBlock = std::variant<TextBlock, ToolUseBlock, ToolResultBlock>;

void to_json(Json& j, const TextBlock& block);
void from_json(const Json& j, TextBlock& block);

void to_json(Json& j, const ToolUseBlock& block);
void from_json(const Json& j, ToolUseBlock& block);

void to_json(Json& j, const ToolResultBlock& block);
void from_json(const Json& j, ToolResultBlock& block);

void to_json(Json& j, const ContentBlock& block);
void from_json(const Json& j, ContentBlock& block);

} // namespace forgeant

#endif // FORGEANT_TYPES_CONTENT_HPP
