#include <agentforge/tool/tool.hpp>

#include <catch2/catch_test_macros.hpp>

struct WeatherParams {
    std::string city;
    std::string unit;
};

template <>
struct agentforge::ParamSchema<WeatherParams> {
    static agentforge::Json schema() {
        return agentforge::Schema::object()
            .property("city", agentforge::Schema::string().description("The city name").build())
            .property("unit", agentforge::Schema::string().description("Temperature unit").build())
            .required({"city"})
            .build();
    }
};

inline void from_json(const agentforge::Json& j, WeatherParams& params) {
    j.at("city").get_to(params.city);
    if (j.contains("unit")) {
        j.at("unit").get_to(params.unit);
    }
}

TEST_CASE("ParamSchema specialization generates correct schema", "[params]") {
    auto schema = agentforge::ParamSchema<WeatherParams>::schema();
    REQUIRE(schema["type"] == "object");
    REQUIRE(schema["properties"].contains("city"));
    REQUIRE(schema["properties"].contains("unit"));
    REQUIRE(schema["properties"]["city"]["description"] == "The city name");
    REQUIRE(schema["required"][0] == "city");
}

TEST_CASE("ParamSchema with from_json enables deserialization", "[params]") {
    agentforge::Json j = {{"city", "Denver"}, {"unit", "fahrenheit"}};
    auto params = j.get<WeatherParams>();
    REQUIRE(params.city == "Denver");
    REQUIRE(params.unit == "fahrenheit");
}

TEST_CASE("ParamSchema works with make_tool", "[params]") {
    auto tool = agentforge::make_tool<WeatherParams>(
        "weather", "Get weather", [](WeatherParams p) { return p.city + ": 72F " + p.unit; });

    auto result = tool.execute({{"city", "Denver"}, {"unit", "fahrenheit"}});
    REQUIRE(result == "Denver: 72F fahrenheit");
}
