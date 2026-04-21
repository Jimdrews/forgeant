#ifndef AGENTFORGE_JSON_JSON_HPP
#define AGENTFORGE_JSON_JSON_HPP

#include <cstdint>
#include <initializer_list>
#include <map>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace agentforge {

namespace detail {

template <typename T>
struct IsVector : std::false_type {};

template <typename T, typename A>
struct IsVector<std::vector<T, A>> : std::true_type {};

} // namespace detail

class Json {
  public:
    using null_t = std::nullptr_t;
    using boolean_t = bool;
    using integer_t = std::int64_t;
    using number_t = double;
    using string_t = std::string;
    using array_t = std::vector<Json>;
    using object_t = std::map<std::string, Json>;

    Json() : value_(nullptr) {}
    Json(std::nullptr_t) : value_(nullptr) {}

    template <typename T>
        requires std::same_as<T, bool>
    Json(T b) : value_(boolean_t{b}) {}

    template <typename T>
        requires(std::integral<T> && !std::same_as<T, bool>)
    Json(T n) : value_(static_cast<integer_t>(n)) {}

    template <typename T>
        requires std::floating_point<T>
    Json(T d) : value_(static_cast<number_t>(d)) {}

    Json(const char* s) : value_(string_t(s)) {}
    Json(std::string_view sv) : value_(string_t(sv)) {}
    Json(string_t s) : value_(std::move(s)) {}
    Json(array_t arr) : value_(std::move(arr)) {}
    Json(object_t obj) : value_(std::move(obj)) {}

    Json(const Json&) = default;
    Json(Json&&) noexcept = default;
    Json& operator=(const Json&) = default;
    Json& operator=(Json&&) noexcept = default;
    ~Json() = default;

    [[nodiscard]] bool is_null() const { return std::holds_alternative<null_t>(value_); }
    [[nodiscard]] bool is_boolean() const { return std::holds_alternative<boolean_t>(value_); }

    [[nodiscard]] bool is_number() const {
        return std::holds_alternative<integer_t>(value_) ||
               std::holds_alternative<number_t>(value_);
    }

    [[nodiscard]] bool is_string() const { return std::holds_alternative<string_t>(value_); }
    [[nodiscard]] bool is_array() const { return std::holds_alternative<array_t>(value_); }
    [[nodiscard]] bool is_object() const { return std::holds_alternative<object_t>(value_); }

    Json& operator[](const std::string& key) {
        if (is_null()) {
            value_ = object_t{};
        }
        return std::get<object_t>(value_)[key];
    }

    const Json& operator[](const std::string& key) const {
        return std::get<object_t>(value_).at(key);
    }

    Json& operator[](std::size_t index) { return std::get<array_t>(value_)[index]; }

    const Json& operator[](std::size_t index) const { return std::get<array_t>(value_)[index]; }

    Json& at(const std::string& key) {
        if (!is_object()) {
            throw std::out_of_range("Json::at: not an object");
        }
        auto& obj = std::get<object_t>(value_);
        auto it = obj.find(key);
        if (it == obj.end()) {
            throw std::out_of_range("Json::at: key not found: " + key);
        }
        return it->second;
    }

    [[nodiscard]] const Json& at(const std::string& key) const {
        if (!is_object()) {
            throw std::out_of_range("Json::at: not an object");
        }
        const auto& obj = std::get<object_t>(value_);
        auto it = obj.find(key);
        if (it == obj.end()) {
            throw std::out_of_range("Json::at: key not found: " + key);
        }
        return it->second;
    }

    template <typename T>
    [[nodiscard]] T value(const std::string& key, const T& default_value) const {
        if (!is_object()) {
            return default_value;
        }
        const auto& obj = std::get<object_t>(value_);
        auto it = obj.find(key);
        if (it == obj.end()) {
            return default_value;
        }
        return it->second.get<T>();
    }

    [[nodiscard]] std::string value(const std::string& key, const char* default_value) const {
        return value<std::string>(key, std::string(default_value));
    }

    [[nodiscard]] bool contains(const std::string& key) const {
        if (!is_object()) {
            return false;
        }
        return std::get<object_t>(value_).contains(key);
    }

    void push_back(Json val) {
        if (is_null()) {
            value_ = array_t{};
        }
        std::get<array_t>(value_).push_back(std::move(val));
    }

    [[nodiscard]] bool empty() const {
        if (is_array()) {
            return std::get<array_t>(value_).empty();
        }
        if (is_object()) {
            return std::get<object_t>(value_).empty();
        }
        if (is_null()) {
            return true;
        }
        return false;
    }

    [[nodiscard]] std::size_t size() const {
        if (is_array()) {
            return std::get<array_t>(value_).size();
        }
        if (is_object()) {
            return std::get<object_t>(value_).size();
        }
        return 0;
    }

    using iterator = array_t::iterator;
    using const_iterator = array_t::const_iterator;

    [[nodiscard]] iterator begin() { return std::get<array_t>(value_).begin(); }
    [[nodiscard]] iterator end() { return std::get<array_t>(value_).end(); }
    [[nodiscard]] const_iterator begin() const { return std::get<array_t>(value_).begin(); }
    [[nodiscard]] const_iterator end() const { return std::get<array_t>(value_).end(); }

    template <typename T>
    [[nodiscard]] T get() const {
        using U = std::remove_cvref_t<T>;
        if constexpr (std::is_same_v<U, Json>) {
            return *this;
        } else if constexpr (std::is_same_v<U, string_t>) {
            return std::get<string_t>(value_);
        } else if constexpr (std::is_same_v<U, bool>) {
            return std::get<boolean_t>(value_);
        } else if constexpr (std::is_same_v<U, integer_t>) {
            return std::get<integer_t>(value_);
        } else if constexpr (std::is_integral_v<U> && !std::is_same_v<U, bool>) {
            return static_cast<U>(std::get<integer_t>(value_));
        } else if constexpr (std::is_floating_point_v<U>) {
            if (std::holds_alternative<integer_t>(value_)) {
                return static_cast<U>(std::get<integer_t>(value_));
            }
            return static_cast<U>(std::get<number_t>(value_));
        } else if constexpr (detail::IsVector<U>::value) {
            using elem_t = typename U::value_type;
            const auto& arr = std::get<array_t>(value_);
            U result;
            result.reserve(arr.size());
            for (const auto& elem : arr) {
                result.push_back(elem.get<elem_t>());
            }
            return result;
        } else {
            U result{};
            from_json(*this, result);
            return result;
        }
    }

    template <typename T>
    void get_to(T& val) const {
        val = get<T>();
    }

    [[nodiscard]] bool operator==(const Json& other) const { return value_ == other.value_; }

    [[nodiscard]] std::string dump() const;
    static Json parse(std::string_view input);

    static Json array() { return Json(array_t{}); }

    static Json array(std::initializer_list<Json> init) {
        array_t arr;
        arr.reserve(init.size());
        for (const auto& elem : init) {
            arr.push_back(elem);
        }
        return Json(std::move(arr));
    }
    static Json object() { return Json(object_t{}); }

    static Json object(std::initializer_list<std::pair<const std::string, Json>> init) {
        object_t obj;
        for (const auto& [key, val] : init) {
            obj.emplace(key, val);
        }
        return Json(std::move(obj));
    }

  private:
    void write_to(std::string& out) const;

    std::variant<null_t, boolean_t, integer_t, number_t, string_t, array_t, object_t> value_;
};

namespace json {

Json parse(std::string_view input);
std::string dump(const Json& j);
Json array();
Json object();

} // namespace json

} // namespace agentforge

#endif // AGENTFORGE_JSON_JSON_HPP
