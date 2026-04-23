#ifndef FORGEANT_HTTP_RESPONSE_HPP
#define FORGEANT_HTTP_RESPONSE_HPP

#include <string>
#include <unordered_map>

namespace forgeant {

/**
 * @ingroup transport
 * @brief Status code, headers, and body of an HTTP response.
 */
struct HttpResponse {
    int status_code = 0;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    bool operator==(const HttpResponse&) const = default;
};

} // namespace forgeant

#endif // FORGEANT_HTTP_RESPONSE_HPP
