#ifndef AGENTFORGE_PROVIDER_CHAT_REQUEST_HPP
#define AGENTFORGE_PROVIDER_CHAT_REQUEST_HPP

#include <nlohmann/json.hpp>
#include <optional>
#include <span>

namespace agentforge {

struct ChatRequest {
    std::span<const nlohmann::json> tools;
    std::optional<nlohmann::json> output_schema;
};

} // namespace agentforge

#endif // AGENTFORGE_PROVIDER_CHAT_REQUEST_HPP
