#include <agentforge/json/json.hpp>

#include <array>
#include <charconv>
#include <cmath>
#include <cstdio>
#include <glaze/json/generic.hpp>
#include <glaze/json/read.hpp>
#include <limits>
#include <stdexcept>

namespace agentforge {

namespace {

Json from_generic(const glz::generic& g) {
    return std::visit(
        [](const auto& val) -> Json {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::nullptr_t>) {
                return {nullptr};
            } else if constexpr (std::is_same_v<T, bool>) {
                bool b = val;
                return {b};
            } else if constexpr (std::is_same_v<T, double>) {
                if (val == std::floor(val) && std::isfinite(val) &&
                    val >= static_cast<double>(std::numeric_limits<Json::integer_t>::min()) &&
                    val <= static_cast<double>(std::numeric_limits<Json::integer_t>::max())) {
                    return Json{static_cast<Json::integer_t>(val)};
                }
                return Json{val};
            } else if constexpr (std::is_same_v<T, std::string>) {
                return Json{val};
            } else if constexpr (std::is_same_v<T, glz::generic::array_t>) {
                Json::array_t arr;
                arr.reserve(val.size());
                for (const auto& elem : val) {
                    arr.push_back(from_generic(elem));
                }
                return {std::move(arr)};
            } else if constexpr (std::is_same_v<T, glz::generic::object_t>) {
                Json::object_t obj;
                for (const auto& [key, elem] : val) {
                    obj.emplace(key, from_generic(elem));
                }
                return {std::move(obj)};
            }
        },
        g.data);
}

void write_string(std::string& out, const std::string& s) {
    out += '"';
    for (char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                std::array<char, 8> buf{};
                std::snprintf(buf.data(), buf.size(), "\\u%04x", static_cast<unsigned char>(c));
                out += buf.data();
            } else {
                out += c;
            }
        }
    }
    out += '"';
}

} // namespace

void Json::write_to(std::string& out) const {
    std::visit(
        [this, &out](const auto& val) {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, null_t>) {
                out += "null";
            } else if constexpr (std::is_same_v<T, boolean_t>) {
                out += val ? "true" : "false";
            } else if constexpr (std::is_same_v<T, integer_t>) {
                std::array<char, 32> buf{};
                auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val);
                out.append(buf.data(), ptr);
            } else if constexpr (std::is_same_v<T, number_t>) {
                std::array<char, 32> buf{};
                auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), val,
                                               std::chars_format::general);
                out.append(buf.data(), ptr);
            } else if constexpr (std::is_same_v<T, string_t>) {
                write_string(out, val);
            } else if constexpr (std::is_same_v<T, array_t>) {
                out += '[';
                bool first = true;
                for (const auto& elem : val) {
                    if (!first) {
                        out += ',';
                    }
                    elem.write_to(out);
                    first = false;
                }
                out += ']';
            } else if constexpr (std::is_same_v<T, object_t>) {
                out += '{';
                bool first = true;
                for (const auto& [key, elem] : val) {
                    if (!first) {
                        out += ',';
                    }
                    write_string(out, key);
                    out += ':';
                    elem.write_to(out);
                    first = false;
                }
                out += '}';
            }
        },
        value_);
}

std::string Json::dump() const {
    std::string out;
    write_to(out);
    return out;
}

Json Json::parse(std::string_view input) {
    auto result = glz::read_json<glz::generic>(input);
    if (result) {
        return from_generic(*result);
    }
    throw std::runtime_error("Json::parse: " + glz::format_error(result.error(), input));
}

namespace json {

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

} // namespace json

} // namespace agentforge
