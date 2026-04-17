#ifndef AGENTFORGE_HTTP_CURL_CLIENT_HPP
#define AGENTFORGE_HTTP_CURL_CLIENT_HPP

#include <agentforge/http/client.hpp>

#include <chrono>

namespace agentforge {

class CurlHttpClient : public HttpClient {
  public:
    struct Config {
        std::chrono::seconds connect_timeout{30};
        std::chrono::seconds transfer_timeout{300};
    };

    CurlHttpClient();
    explicit CurlHttpClient(Config config);
    ~CurlHttpClient() override;

    CurlHttpClient(const CurlHttpClient&) = delete;
    CurlHttpClient& operator=(const CurlHttpClient&) = delete;
    CurlHttpClient(CurlHttpClient&& other) noexcept;
    CurlHttpClient& operator=(CurlHttpClient&& other) noexcept;

    HttpResponse post(const std::string& url, const HttpHeaders& headers,
                      const std::string& body) override;

    std::future<HttpResponse> async_post(const std::string& url, const HttpHeaders& headers,
                                         const std::string& body) override;

    HttpResponse post_stream(const std::string& url, const HttpHeaders& headers,
                             const std::string& body, StreamCallback on_chunk) override;

  private:
    struct Impl;
    Impl* impl_;
    Config config_;

    HttpResponse perform_post(void* curl, const std::string& url, const HttpHeaders& headers,
                              const std::string& body, StreamCallback* on_chunk) const;
};

} // namespace agentforge

#endif // AGENTFORGE_HTTP_CURL_CLIENT_HPP
