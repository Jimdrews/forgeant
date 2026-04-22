#ifndef FORGEANT_PROVIDER_ANTHROPIC_HPP
#define FORGEANT_PROVIDER_ANTHROPIC_HPP

#include <forgeant/http/client.hpp>
#include <forgeant/provider/config.hpp>
#include <forgeant/provider/provider.hpp>

namespace forgeant {

class AnthropicProvider : public LlmProvider {
  public:
    AnthropicProvider(HttpClient& client, ProviderConfig config);

    LlmResponse chat(const Conversation& conversation, const ChatRequest& request = {}) override;

  private:
    HttpClient& client_;
    ProviderConfig config_;

    static constexpr const char* DEFAULT_BASE_URL = "https://api.anthropic.com";
    static constexpr const char* API_VERSION = "2023-06-01";

    [[nodiscard]] Json serialize_request(const Conversation& conversation,
                                         const ChatRequest& request) const;
    [[nodiscard]] static LlmResponse deserialize_response(const Json& json);
    [[nodiscard]] std::string endpoint_url() const;
    [[nodiscard]] HttpHeaders auth_headers() const;
};

} // namespace forgeant

#endif // FORGEANT_PROVIDER_ANTHROPIC_HPP
