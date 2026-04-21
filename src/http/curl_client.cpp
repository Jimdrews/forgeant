#include <agentforge/http/curl_client.hpp>

#include <curl/curl.h>
#include <mutex>
#include <stdexcept>

namespace agentforge {

namespace {

struct CurlGlobalGuard {
    CurlGlobalGuard() { curl_global_init(CURL_GLOBAL_DEFAULT); }
    ~CurlGlobalGuard() { curl_global_cleanup(); }
    CurlGlobalGuard(const CurlGlobalGuard&) = delete;
    CurlGlobalGuard& operator=(const CurlGlobalGuard&) = delete;
};

void ensure_curl_initialized() {
    static CurlGlobalGuard guard;
}

struct WriteContext {
    std::string* body;
    StreamCallback* on_chunk;
    bool aborted = false;
};

size_t write_callback(char* ptr, size_t size, size_t nmemb, void* userdata) {
    auto total = size * nmemb;
    auto* ctx = static_cast<WriteContext*>(userdata);

    if (ctx->on_chunk != nullptr) {
        if (!(*ctx->on_chunk)(std::string_view(ptr, total))) {
            ctx->aborted = true;
            return 0;
        }
        return total;
    }

    ctx->body->append(ptr, total);
    return total;
}

size_t header_callback(char* buffer, size_t size, size_t nitems, void* userdata) {
    auto total = size * nitems;
    auto* headers = static_cast<std::unordered_map<std::string, std::string>*>(userdata);

    std::string_view line(buffer, total);
    if (auto colon = line.find(':'); colon != std::string_view::npos) {
        auto key = std::string(line.substr(0, colon));
        auto value_start = line.find_first_not_of(' ', colon + 1);
        auto value_end = line.find_last_not_of("\r\n");
        if (value_start != std::string_view::npos && value_end != std::string_view::npos) {
            headers->emplace(std::move(key),
                             std::string(line.substr(value_start, value_end - value_start + 1)));
        }
    }

    return total;
}

} // namespace

struct CurlHttpClient::Impl {
    CURL* handle = nullptr;
};

CurlHttpClient::CurlHttpClient() : CurlHttpClient(Config{}) {}

CurlHttpClient::CurlHttpClient(Config config) : impl_(new Impl), config_(config) {
    ensure_curl_initialized();
    impl_->handle = curl_easy_init();
    if (impl_->handle == nullptr) {
        delete impl_;
        throw std::runtime_error("failed to initialize curl handle");
    }
}

CurlHttpClient::~CurlHttpClient() {
    if (impl_ != nullptr) {
        if (impl_->handle != nullptr) {
            curl_easy_cleanup(impl_->handle);
        }
        delete impl_;
    }
}

CurlHttpClient::CurlHttpClient(CurlHttpClient&& other) noexcept
    : impl_(other.impl_), config_(other.config_) {
    other.impl_ = nullptr;
}

CurlHttpClient& CurlHttpClient::operator=(CurlHttpClient&& other) noexcept {
    if (this != &other) {
        if (impl_ != nullptr) {
            if (impl_->handle != nullptr) {
                curl_easy_cleanup(impl_->handle);
            }
            delete impl_;
        }
        impl_ = other.impl_;
        config_ = other.config_;
        other.impl_ = nullptr;
    }
    return *this;
}

HttpResponse CurlHttpClient::perform_post(void* curl_handle, const std::string& url,
                                          const HttpHeaders& headers, const std::string& body,
                                          StreamCallback* on_chunk) const {
    auto* curl = static_cast<CURL*>(curl_handle);

    curl_easy_reset(curl);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,
                     static_cast<long>(config_.connect_timeout.count()));
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(config_.transfer_timeout.count()));

    struct curl_slist* header_list = nullptr;
    for (const auto& [key, value] : headers) {
        std::string header;
        header.reserve(key.size() + 2 + value.size());
        header += key;
        header += ": ";
        header += value;
        header_list = curl_slist_append(header_list, header.c_str());
    }
    if (header_list != nullptr) {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    }

    HttpResponse response;
    std::string response_body;
    WriteContext write_ctx{.body = &response_body, .on_chunk = on_chunk};

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &write_ctx);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &response.headers);

    CURLcode res = curl_easy_perform(curl);

    if (header_list != nullptr) {
        curl_slist_free_all(header_list);
    }

    if (res != CURLE_OK && !write_ctx.aborted) {
        throw std::runtime_error(std::string("HTTP request failed: ") + curl_easy_strerror(res));
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    response.status_code = static_cast<int>(http_code);

    if (on_chunk == nullptr) {
        response.body = std::move(response_body);
    }

    return response;
}

HttpResponse CurlHttpClient::post(const std::string& url, const HttpHeaders& headers,
                                  const std::string& body) {
    return perform_post(impl_->handle, url, headers, body, nullptr);
}

std::future<HttpResponse> CurlHttpClient::async_post(const std::string& url,
                                                     const HttpHeaders& headers,
                                                     const std::string& body) {
    auto config = config_;
    return std::async(std::launch::async, [url, headers, body, config]() {
        CurlHttpClient client(config);
        return client.post(url, headers, body);
    });
}

HttpResponse CurlHttpClient::post_stream(const std::string& url, const HttpHeaders& headers,
                                         const std::string& body, StreamCallback on_chunk) {
    return perform_post(impl_->handle, url, headers, body, &on_chunk);
}

} // namespace agentforge
