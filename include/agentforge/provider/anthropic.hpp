#pragma once

#include <agentforge/http/client.hpp>
#include <agentforge/provider/config.hpp>
#include <agentforge/provider/provider.hpp>

namespace agentforge {

class AnthropicProvider : public LlmProvider {
  public:
    AnthropicProvider(HttpClient& client, ProviderConfig config);

    LlmResponse chat(const Conversation& conversation) override;

  private:
    HttpClient& client_;
    ProviderConfig config_;

    static constexpr const char* DEFAULT_BASE_URL = "https://api.anthropic.com";
    static constexpr const char* API_VERSION = "2023-06-01";

    [[nodiscard]] nlohmann::json serialize_request(const Conversation& conversation) const;
    [[nodiscard]] static LlmResponse deserialize_response(const nlohmann::json& json);
    [[nodiscard]] std::string endpoint_url() const;
    [[nodiscard]] HttpHeaders auth_headers() const;
};

} // namespace agentforge
