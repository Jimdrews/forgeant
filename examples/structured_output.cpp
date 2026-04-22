#include <cstdlib>
#include <forgeant/forgeant.hpp>
#include <iostream>

struct MovieReview {
    std::string title;
    int rating = 0;
    std::string summary;
};

template <>
struct forgeant::ParamSchema<MovieReview> {
    static forgeant::Json schema() {
        return forgeant::Schema::object()
            .property("title", forgeant::Schema::string().description("Movie title").build())
            .property("rating",
                      forgeant::Schema::integer().description("Rating from 1 to 10").build())
            .property("summary",
                      forgeant::Schema::string().description("One sentence summary").build())
            .required({"title", "rating", "summary"})
            .build();
    }
};

void from_json(const forgeant::Json& j, MovieReview& review) {
    j.at("title").get_to(review.title);
    j.at("rating").get_to(review.rating);
    j.at("summary").get_to(review.summary);
}

int main() {
    try {
        const char* provider = std::getenv("FORGEANT_PROVIDER");
        const char* model = std::getenv("FORGEANT_MODEL");
        const char* api_key = std::getenv("FORGEANT_API_KEY");

        auto agent = forgeant::Agent::create(provider != nullptr ? provider : "ollama",
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
