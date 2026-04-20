#ifndef AGENTFORGE_JSON_JSON_HPP
#define AGENTFORGE_JSON_JSON_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <string_view>

namespace agentforge {

using Json = nlohmann::json;

namespace json {

Json parse(std::string_view input);
std::string dump(const Json& j);

template <typename T>
T get(const Json& j) {
    return j.get<T>();
}

template <typename T>
Json from(const T& value) {
    return Json(value);
}

Json array();
Json object();

} // namespace json

} // namespace agentforge

#endif // AGENTFORGE_JSON_JSON_HPP
