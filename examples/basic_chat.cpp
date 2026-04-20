#include <agentforge/agentforge.hpp>

#include <cstdlib>
#include <iostream>

int main() {
    try {
        const char* provider = std::getenv("AGENTFORGE_PROVIDER");
        const char* model = std::getenv("AGENTFORGE_MODEL");
        const char* api_key = std::getenv("AGENTFORGE_API_KEY");

        auto agent = agentforge::Agent::create(
            provider != nullptr ? provider : "ollama",
            {
                .api_key = api_key != nullptr ? api_key : "",
                .model = model != nullptr ? model : "llama3",
                .system_prompt = "You are a helpful assistant. Be concise.",
            });

        auto result = agent->run("What is the capital of France? Answer in one sentence.");

        std::cout << "Response: " << result.output << std::endl;
        std::cout << "Tokens: " << result.total_usage.input_tokens << " in, "
                  << result.total_usage.output_tokens << " out" << std::endl;
        std::cout << "Iterations: " << result.iterations << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
