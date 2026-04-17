#pragma once

#include <optional>
#include <string>

namespace agentforge {

struct ProviderConfig {
    std::string api_key;
    std::string model;
    std::string base_url;
    std::optional<int> max_tokens;
    std::optional<double> temperature;
};

} // namespace agentforge
