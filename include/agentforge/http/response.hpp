#pragma once

#include <string>
#include <unordered_map>

namespace agentforge {

struct HttpResponse {
    int status_code = 0;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    bool operator==(const HttpResponse&) const = default;
};

} // namespace agentforge
