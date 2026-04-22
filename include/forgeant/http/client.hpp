#ifndef FORGEANT_HTTP_CLIENT_HPP
#define FORGEANT_HTTP_CLIENT_HPP

#include <forgeant/http/response.hpp>
#include <functional>
#include <future>
#include <string>
#include <string_view>
#include <unordered_map>

namespace forgeant {

using HttpHeaders = std::unordered_map<std::string, std::string>;
using StreamCallback = std::function<bool(std::string_view)>;

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
