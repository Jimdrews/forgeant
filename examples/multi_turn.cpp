#include <cstdlib>
#include <forgeant/forgeant.hpp>
#include <iostream>

int main() {
    const char* provider = std::getenv("FORGEANT_PROVIDER");
    const char* model = std::getenv("FORGEANT_MODEL");
    const char* api_key = std::getenv("FORGEANT_API_KEY");

    try {
        auto agent =
            forgeant::Agent::create(provider != nullptr ? provider : "ollama",
                                    {
                                        .api_key = api_key != nullptr ? api_key : "",
                                        .model = model != nullptr ? model : "llama3",
                                        .system_prompt = "You are a helpful assistant. Be concise.",
                                    });

        std::cout << "=== Turn 1 ===" << std::endl;
        std::cout << "User: My name is Alice. Remember that." << std::endl;
        auto r1 = agent->run("My name is Alice. Remember that.");
        std::cout << "Assistant: " << r1.output << std::endl;

        std::cout << "\n=== Turn 2 ===" << std::endl;
        std::cout << "User: What is my name?" << std::endl;
        forgeant::Conversation conv = r1.conversation;
        conv.add(forgeant::Message(forgeant::Role::user, "What is my name?"));
        auto r2 = agent->run(conv);
        std::cout << "Assistant: " << r2.output << std::endl;

        std::cout << "\n=== Turn 3 ===" << std::endl;
        std::cout << "User: How many messages have we exchanged so far?" << std::endl;
        conv = r2.conversation;
        conv.add(
            forgeant::Message(forgeant::Role::user, "How many messages have we exchanged so far?"));
        auto r3 = agent->run(conv);
        std::cout << "Assistant: " << r3.output << std::endl;

        std::cout << "\nConversation has " << r3.conversation.messages().size() << " messages"
                  << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
