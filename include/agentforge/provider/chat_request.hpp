#ifndef AGENTFORGE_PROVIDER_CHAT_REQUEST_HPP
#define AGENTFORGE_PROVIDER_CHAT_REQUEST_HPP

#include <agentforge/tool/tool_view.hpp>

#include <nlohmann/json.hpp>
#include <optional>
#include <span>

namespace agentforge {

struct ChatRequest {
    std::span<const ToolView> tools;
    std::optional<nlohmann::json> output_schema;
};

} // namespace agentforge

#endif // AGENTFORGE_PROVIDER_CHAT_REQUEST_HPP
