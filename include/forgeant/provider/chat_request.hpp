#ifndef FORGEANT_PROVIDER_CHAT_REQUEST_HPP
#define FORGEANT_PROVIDER_CHAT_REQUEST_HPP

#include <forgeant/json/json.hpp>
#include <forgeant/tool/tool_view.hpp>
#include <optional>
#include <span>

namespace forgeant {

struct ChatRequest {
    std::span<const ToolView> tools;
    std::optional<Json> output_schema;
};

} // namespace forgeant

#endif // FORGEANT_PROVIDER_CHAT_REQUEST_HPP
