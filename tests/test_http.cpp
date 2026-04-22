#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <forgeant/http/curl_client.hpp>
#include <httplib.h>
#include <thread>

namespace {

class TestServer {
  public:
    TestServer() {
        server_.Post("/echo", [](const httplib::Request& req, httplib::Response& res) {
            res.set_content(req.body, req.get_header_value("Content-Type"));
        });

        server_.Post("/headers", [](const httplib::Request& req, httplib::Response& res) {
            std::string result;
            for (const auto& [key, value] : req.headers) {
                result += key + ": " + value + "\n";
            }
            res.set_content(result, "text/plain");
        });

        server_.Post("/stream", [](const httplib::Request& /*req*/, httplib::Response& res) {
            res.set_content("chunk1chunk2chunk3", "text/plain");
        });

        server_.Post("/slow", [](const httplib::Request& /*req*/, httplib::Response& res) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            res.set_content("done", "text/plain");
        });

        port_ = server_.bind_to_any_port("127.0.0.1");
        thread_ = std::thread([this]() { server_.listen_after_bind(); });

        while (!server_.is_running()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    ~TestServer() {
        server_.stop();
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    TestServer(const TestServer&) = delete;
    TestServer& operator=(const TestServer&) = delete;

    [[nodiscard]] std::string url(const std::string& path) const {
        return "http://127.0.0.1:" + std::to_string(port_) + path;
    }

  private:
    httplib::Server server_;
    std::thread thread_;
    int port_ = 0;
};

} // namespace

TEST_CASE("Sync POST returns response", "[http]") {
    TestServer server;
    forgeant::CurlHttpClient client;

    auto response = client.post(server.url("/echo"), {{"Content-Type", "application/json"}},
                                R"({"hello": "world"})");

    REQUIRE(response.status_code == 200);
    REQUIRE(response.body == R"({"hello": "world"})");
}

TEST_CASE("Sync POST sends custom headers", "[http]") {
    TestServer server;
    forgeant::CurlHttpClient client;

    auto response = client.post(server.url("/headers"), {{"X-Custom", "test-value"}}, "");

    REQUIRE(response.status_code == 200);
    REQUIRE(response.body.find("X-Custom: test-value") != std::string::npos);
}

TEST_CASE("Async POST resolves future", "[http]") {
    TestServer server;
    forgeant::CurlHttpClient client;

    auto future =
        client.async_post(server.url("/echo"), {{"Content-Type", "text/plain"}}, "async body");

    auto response = future.get();
    REQUIRE(response.status_code == 200);
    REQUIRE(response.body == "async body");
}

TEST_CASE("Streaming POST invokes callback", "[http]") {
    TestServer server;
    forgeant::CurlHttpClient client;

    std::string accumulated;
    auto response = client.post_stream(server.url("/stream"), {{"Content-Type", "text/plain"}}, "",
                                       [&accumulated](std::string_view chunk) {
                                           accumulated.append(chunk);
                                           return true;
                                       });

    REQUIRE(response.status_code == 200);
    REQUIRE(response.body.empty());
    REQUIRE(accumulated == "chunk1chunk2chunk3");
}

TEST_CASE("Streaming POST abort via callback", "[http]") {
    TestServer server;
    forgeant::CurlHttpClient client;

    int call_count = 0;
    client.post_stream(server.url("/stream"), {}, "", [&call_count](std::string_view /*chunk*/) {
        call_count++;
        return false;
    });

    REQUIRE(call_count == 1);
}

TEST_CASE("Connection refused throws", "[http]") {
    forgeant::CurlHttpClient client;

    REQUIRE_THROWS_AS(client.post("http://127.0.0.1:1", {}, ""), std::runtime_error);
}

TEST_CASE("Timeout on slow server", "[http]") {
    TestServer server;
    forgeant::CurlHttpClient::Config config;
    config.transfer_timeout = std::chrono::seconds(1);
    forgeant::CurlHttpClient client(config);

    REQUIRE_THROWS_AS(client.post(server.url("/slow"), {}, ""), std::runtime_error);
}
