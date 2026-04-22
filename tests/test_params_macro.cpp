#include <catch2/catch_test_macros.hpp>

#include <forgeant/tool/tool.hpp>

struct WeatherParams {
    std::string city;
    std::string unit;
};

template <>
struct forgeant::ParamSchema<WeatherParams> {
    static forgeant::Json schema() {
        return forgeant::Schema::object()
            .property("city", forgeant::Schema::string().description("The city name").build())
            .property("unit", forgeant::Schema::string().description("Temperature unit").build())
            .required({"city"})
            .build();
    }
};

inline void from_json(const forgeant::Json& j, WeatherParams& params) {
    j.at("city").get_to(params.city);
    if (j.contains("unit")) {
        j.at("unit").get_to(params.unit);
    }
}

TEST_CASE("ParamSchema specialization generates correct schema", "[params]") {
    auto schema = forgeant::ParamSchema<WeatherParams>::schema();
    REQUIRE(schema["type"] == "object");
    REQUIRE(schema["properties"].contains("city"));
    REQUIRE(schema["properties"].contains("unit"));
    REQUIRE(schema["properties"]["city"]["description"] == "The city name");
    REQUIRE(schema["required"][0] == "city");
}

TEST_CASE("ParamSchema with from_json enables deserialization", "[params]") {
    forgeant::Json j = forgeant::Json::object({{"city", "Denver"}, {"unit", "fahrenheit"}});
    auto params = j.get<WeatherParams>();
    REQUIRE(params.city == "Denver");
    REQUIRE(params.unit == "fahrenheit");
}

TEST_CASE("ParamSchema works with make_tool", "[params]") {
    auto tool = forgeant::make_tool<WeatherParams>(
        "weather", "Get weather", [](WeatherParams p) { return p.city + ": 72F " + p.unit; });

    auto result =
        tool.execute(forgeant::Json::object({{"city", "Denver"}, {"unit", "fahrenheit"}}));
    REQUIRE(result == "Denver: 72F fahrenheit");
}
