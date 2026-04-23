#ifndef FORGEANT_PROVIDER_CONFIG_HPP
#define FORGEANT_PROVIDER_CONFIG_HPP

#include <optional>
#include <string>

namespace forgeant {

/**
 * @ingroup providers
 * @brief Per-provider configuration owned directly by an `LlmProvider` (credentials, model, URL,
 * sampling).
 */
struct ProviderConfig {
    std::string api_key;
    std::string model;
    std::string base_url;
    std::optional<int> max_tokens;
    std::optional<double> temperature;
};

} // namespace forgeant

#endif // FORGEANT_PROVIDER_CONFIG_HPP
