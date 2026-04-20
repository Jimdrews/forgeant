#include <agentforge/json/json.hpp>

namespace agentforge::json {

Json parse(std::string_view input) {
    return Json::parse(input);
}

std::string dump(const Json& j) {
    return j.dump();
}

Json array() {
    return Json::array();
}

Json object() {
    return Json::object();
}

} // namespace agentforge::json
