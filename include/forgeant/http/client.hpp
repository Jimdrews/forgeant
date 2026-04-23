#ifndef FORGEANT_HTTP_CLIENT_HPP
#define FORGEANT_HTTP_CLIENT_HPP

#include <forgeant/http/response.hpp>
#include <functional>
#include <future>
#include <string>
#include <string_view>
#include <unordered_map>

namespace forgeant {

/**
 * @ingroup transport
 * @brief Case-insensitive-in-spirit map of HTTP header name to value.
 */
using HttpHeaders = std::unordered_map<std::string, std::string>;

/**
 * @ingroup transport
 * @brief Callback invoked for each streamed response chunk; return `false` to stop.
 */
using StreamCallback = std::function<bool(std::string_view)>;

/**
 * @ingroup transport
 * @brief Abstract HTTP client (POST, async POST, streaming POST); lets providers be mocked in
 * tests.
 */
class HttpClient {
  public:
    virtual ~HttpClient() = default;

    virtual HttpResponse post(const std::string& url, const HttpHeaders& headers,
                              const std::string& body) = 0;

    virtual std::future<HttpResponse> async_post(const std::string& url, const HttpHeaders& headers,
                                                 const std::string& body) = 0;

    virtual HttpResponse post_stream(const std::string& url, const HttpHeaders& headers,
                                     const std::string& body, StreamCallback on_chunk) = 0;

    HttpClient() = default;
    HttpClient(const HttpClient&) = delete;
    HttpClient& operator=(const HttpClient&) = delete;
    HttpClient(HttpClient&&) = default;
    HttpClient& operator=(HttpClient&&) = default;
};

} // namespace forgeant

#endif // FORGEANT_HTTP_CLIENT_HPP
