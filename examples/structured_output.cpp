#include <agentforge/agentforge.hpp>

#include <cstdlib>
#include <iostream>

struct MovieReview {
    std::string title;
    int rating = 0;
    std::string summary;
};

template <>
struct agentforge::ParamSchema<MovieReview> {
    static agentforge::Json schema() {
        return agentforge::Schema::object()
            .property("title", agentforge::Schema::string().description("Movie title").build())
            .property("rating",
                      agentforge::Schema::integer().description("Rating from 1 to 10").build())
            .property("summary",
                      agentforge::Schema::string().description("One sentence summary").build())
            .required({"title", "rating", "summary"})
            .build();
    }
};

void from_json(const agentforge::Json& j, MovieReview& review) {
    j.at("title").get_to(review.title);
    j.at("rating").get_to(review.rating);
    j.at("summary").get_to(review.summary);
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
                                               });

        auto result = agent->run<MovieReview>("Review the movie 'The Matrix' (1999).");

        std::cout << "Title: " << result.output.title << std::endl;
        std::cout << "Rating: " << result.output.rating << "/10" << std::endl;
        std::cout << "Summary: " << result.output.summary << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
