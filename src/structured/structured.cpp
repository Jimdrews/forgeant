#include <forgeant/structured/structured.hpp>
#include <stdexcept>
#include <variant>

namespace forgeant {

Json extract_json_from_response(const LlmResponse& response) {
    for (const auto& block : response.message.content) {
        if (std::holds_alternative<TextBlock>(block)) {
            return Json::parse(std::get<TextBlock>(block).text);
        }
    }
    throw std::runtime_error("structured output response contains no text content");
}

} // namespace forgeant
