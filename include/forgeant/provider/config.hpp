#ifndef FORGEANT_PROVIDER_CONFIG_HPP
#define FORGEANT_PROVIDER_CONFIG_HPP

#include <optional>
#include <string>

namespace forgeant {

struct ProviderConfig {
    std::string api_key;
    std::string model;
    std::string base_url;
    std::optional<int> max_tokens;
    std::optional<double> temperature;
};

} // namespace forgeant

#endif // FORGEANT_PROVIDER_CONFIG_HPP
