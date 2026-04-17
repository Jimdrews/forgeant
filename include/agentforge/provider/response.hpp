#pragma once

#include <agentforge/types/message.hpp>

#include <string>

namespace agentforge {

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

} // namespace agentforge
