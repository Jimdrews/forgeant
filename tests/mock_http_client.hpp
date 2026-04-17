#pragma once

#include <agentforge/http/client.hpp>

#include <functional>
#include <stdexcept>

namespace agentforge::testing {

class MockHttpClient : public HttpClient {
  public:
    HttpResponse canned_response;
    std::function<void(const std::string&, const HttpHeaders&, const std::string&)> on_post;

    HttpResponse post(const std::string& url, const HttpHeaders& headers,
                      const std::string& body) override {
        if (on_post) {
            on_post(url, headers, body);
        }
        return canned_response;
    }

    std::future<HttpResponse> async_post(const std::string& url, const HttpHeaders& headers,
                                         const std::string& body) override {
        std::promise<HttpResponse> promise;
        promise.set_value(post(url, headers, body));
        return promise.get_future();
    }

    HttpResponse post_stream(const std::string& url, const HttpHeaders& headers,
                             const std::string& body, StreamCallback /*on_chunk*/) override {
        return post(url, headers, body);
    }
};

} // namespace agentforge::testing
