#ifndef AGENTFORGE_PROVIDER_OPENAI_HPP
#define AGENTFORGE_PROVIDER_OPENAI_HPP

#include <agentforge/http/client.hpp>
#include <agentforge/provider/config.hpp>
#include <agentforge/provider/provider.hpp>

namespace agentforge {

class OpenAiProvider : public LlmProvider {
  public:
    OpenAiProvider(HttpClient& client, ProviderConfig config);

    LlmResponse chat(const Conversation& conversation, const ChatRequest& request = {}) override;

  private:
    HttpClient& client_;
    ProviderConfig config_;

    static constexpr const char* DEFAULT_BASE_URL = "https://api.openai.com";

    [[nodiscard]] nlohmann::json serialize_request(const Conversation& conversation,
                                                   const ChatRequest& request) const;
    [[nodiscard]] static LlmResponse deserialize_response(const nlohmann::json& json);
    [[nodiscard]] std::string endpoint_url() const;
    [[nodiscard]] HttpHeaders auth_headers() const;
};

} // namespace agentforge

#endif // AGENTFORGE_PROVIDER_OPENAI_HPP
