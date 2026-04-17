#include <agentforge/http/curl_client.hpp>

#include <benchmark/benchmark.h>
#include <httplib.h>
#include <thread>

namespace {

class BenchServer {
  public:
    BenchServer() {
        server_.Post("/echo", [](const httplib::Request& req, httplib::Response& res) {
            res.set_content(req.body, "text/plain");
        });

        port_ = server_.bind_to_any_port("127.0.0.1");
        thread_ = std::thread([this]() { server_.listen_after_bind(); });

        while (!server_.is_running()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    ~BenchServer() {
        server_.stop();
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    BenchServer(const BenchServer&) = delete;
    BenchServer& operator=(const BenchServer&) = delete;

    [[nodiscard]] std::string url() const { return "http://127.0.0.1:" + std::to_string(port_); }

  private:
    httplib::Server server_;
    std::thread thread_;
    int port_ = 0;
};

BenchServer& get_server() {
    static BenchServer server;
    return server;
}

} // namespace

static void BM_HttpPostSync(benchmark::State& state) {
    auto& server = get_server();
    agentforge::CurlHttpClient client;
    std::string body = R"({"message": "hello"})";

    for (auto _ : state) {
        auto response =
            client.post(server.url() + "/echo", {{"Content-Type", "application/json"}}, body);
        benchmark::DoNotOptimize(response);
    }
}
BENCHMARK(BM_HttpPostSync);
