#include <agentforge/structured/structured.hpp>

#include <stdexcept>
#include <variant>

namespace agentforge {

nlohmann::json extract_json_from_response(const LlmResponse& response) {
    for (const auto& block : response.message.content) {
        if (std::holds_alternative<TextBlock>(block)) {
            return nlohmann::json::parse(std::get<TextBlock>(block).text);
        }
    }
    throw std::runtime_error("structured output response contains no text content");
}

} // namespace agentforge
