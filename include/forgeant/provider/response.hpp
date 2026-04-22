#ifndef FORGEANT_PROVIDER_RESPONSE_HPP
#define FORGEANT_PROVIDER_RESPONSE_HPP

#include <forgeant/types/message.hpp>
#include <string>

namespace forgeant {

struct Usage {
    int input_tokens = 0;
    int output_tokens = 0;
};

struct LlmResponse {
    Message message{Role::assistant, ""};
    std::string finish_reason;
    Usage usage;
    std::string model;
};

} // namespace forgeant

#endif // FORGEANT_PROVIDER_RESPONSE_HPP
