#ifndef FORGEANT_PROVIDER_OPENAI_HPP
#define FORGEANT_PROVIDER_OPENAI_HPP

#include <forgeant/http/client.hpp>
#include <forgeant/provider/config.hpp>
#include <forgeant/provider/provider.hpp>

namespace forgeant {

class OpenAiProvider : public LlmProvider {
  public:
    OpenAiProvider(HttpClient& client, ProviderConfig config);

    LlmResponse chat(const Conversation& conversation, const ChatRequest& request = {}) override;

  private:
    HttpClient& client_;
    ProviderConfig config_;

    static constexpr const char* DEFAULT_BASE_URL = "https://api.openai.com";

    [[nodiscard]] Json serialize_request(const Conversation& conversation,
                                         const ChatRequest& request) const;
    [[nodiscard]] static LlmResponse deserialize_response(const Json& json);
    [[nodiscard]] std::string endpoint_url() const;
    [[nodiscard]] HttpHeaders auth_headers() const;
};

} // namespace forgeant

#endif // FORGEANT_PROVIDER_OPENAI_HPP
