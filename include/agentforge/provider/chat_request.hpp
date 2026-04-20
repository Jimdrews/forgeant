#ifndef AGENTFORGE_PROVIDER_CHAT_REQUEST_HPP
#define AGENTFORGE_PROVIDER_CHAT_REQUEST_HPP

#include <agentforge/json/json.hpp>
#include <agentforge/tool/tool_view.hpp>

#include <optional>
#include <span>

namespace agentforge {

struct ChatRequest {
    std::span<const ToolView> tools;
    std::optional<Json> output_schema;
};

} // namespace agentforge

#endif // AGENTFORGE_PROVIDER_CHAT_REQUEST_HPP
