#ifndef FORGEANT_PROVIDER_RESPONSE_HPP
#define FORGEANT_PROVIDER_RESPONSE_HPP

#include <forgeant/types/message.hpp>
#include <string>

namespace forgeant {

/**
 * @ingroup providers
 * @brief Token-accounting for a single provider call or an accumulated run.
 */
struct Usage {
    int input_tokens = 0;
    int output_tokens = 0;
};

/**
 * @ingroup providers
 * @brief A single turn returned by an `LlmProvider`: message, finish reason, usage, and model tag.
 */
struct LlmResponse {
    Message message{Role::assistant, ""};
    std::string finish_reason;
    Usage usage;
    std::string model;
};

} // namespace forgeant

#endif // FORGEANT_PROVIDER_RESPONSE_HPP
