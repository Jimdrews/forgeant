#include <agentforge/agentforge.hpp>

#include <cstdlib>
#include <iostream>

int main() {
    const char* provider = std::getenv("AGENTFORGE_PROVIDER");
    const char* model = std::getenv("AGENTFORGE_MODEL");
    const char* api_key = std::getenv("AGENTFORGE_API_KEY");

    try {
        auto agent = agentforge::Agent::create(
            provider != nullptr ? provider : "ollama",
            {
                .api_key = api_key != nullptr ? api_key : "",
                .model = model != nullptr ? model : "llama3",
                .system_prompt = "You are a helpful assistant. Be concise.",
            });

        agentforge::Conversation conv;

        std::cout << "=== Turn 1 ===" << std::endl;
        std::cout << "User: My name is Alice. Remember that." << std::endl;
        auto r1 = agent->chat(conv, "My name is Alice. Remember that.");
        std::cout << "Assistant: " << r1.text << std::endl;

        std::cout << "\n=== Turn 2 ===" << std::endl;
        std::cout << "User: What is my name?" << std::endl;
        auto r2 = agent->chat(conv, "What is my name?");
        std::cout << "Assistant: " << r2.text << std::endl;

        std::cout << "\n=== Turn 3 ===" << std::endl;
        std::cout << "User: How many messages have we exchanged so far?" << std::endl;
        auto r3 = agent->chat(conv, "How many messages have we exchanged so far?");
        std::cout << "Assistant: " << r3.text << std::endl;

        std::cout << "\nConversation has " << conv.messages().size() << " messages" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
