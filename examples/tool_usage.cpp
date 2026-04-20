#include <agentforge/agentforge.hpp>

#include <cstdlib>
#include <iostream>

struct WeatherParams {
    std::string city;
    std::string unit = "fahrenheit";
};

template <>
struct agentforge::ParamSchema<WeatherParams> {
    static agentforge::Json schema() {
        return agentforge::Schema::object()
            .property("city", agentforge::Schema::string().description("The city name").build())
            .property("unit", agentforge::Schema::string()
                                  .description("Temperature unit")
                                  .enum_values({"fahrenheit", "celsius"})
                                  .default_value("fahrenheit")
                                  .build())
            .required({"city"})
            .build();
    }
};

void from_json(const agentforge::Json& j, WeatherParams& params) {
    j.at("city").get_to(params.city);
    if (j.contains("unit")) {
        j.at("unit").get_to(params.unit);
    }
}

int main() {
    try {
        const char* provider = std::getenv("AGENTFORGE_PROVIDER");
        const char* model = std::getenv("AGENTFORGE_MODEL");
        const char* api_key = std::getenv("AGENTFORGE_API_KEY");

        auto agent = agentforge::Agent::create(provider != nullptr ? provider : "ollama",
                                               {
                                                   .api_key = api_key != nullptr ? api_key : "",
                                                   .model = model != nullptr ? model : "llama3",
                                                   .system_prompt = "You are a helpful assistant.",
                                               });

        agent->add_tool(agentforge::make_tool<WeatherParams>(
            "get_weather", "Get the current weather for a city",
            [](WeatherParams params) -> std::string {
                return "72°F and sunny in " + params.city;
            }));

        auto result = agent->run("What's the weather like in Denver?");

        std::cout << "Response: " << result.output << std::endl;
        std::cout << "Iterations: " << result.iterations << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
