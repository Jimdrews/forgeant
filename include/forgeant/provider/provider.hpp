#ifndef FORGEANT_PROVIDER_PROVIDER_HPP
#define FORGEANT_PROVIDER_PROVIDER_HPP

#include <forgeant/provider/chat_request.hpp>
#include <forgeant/provider/response.hpp>
#include <forgeant/types/conversation.hpp>

namespace forgeant {

/**
 * @ingroup providers
 * @brief Abstract interface for sending a conversation to an LLM backend.
 *
 * `LlmProvider` is a single-method contract: given a `Conversation` and an optional
 * `ChatRequest` (carrying tool definitions and/or a structured-output schema), return an
 * `LlmResponse` with the assistant's next message, finish reason, usage, and model name.
 *
 * ## Usual usage
 *
 * You do not normally construct a provider directly. `Agent::create("anthropic", opts)`
 * (or `"openai"`, `"ollama"`) builds the matching concrete provider behind the scenes
 * from `AgentOptions`. Reach for the provider types directly only when you want to
 * share an `HttpClient` across multiple providers, or inject a fake for testing.
 *
 * ## Implementing a custom provider
 *
 * Implement `chat()` to serialize the conversation to the backend's wire format, perform
 * the HTTP request, and deserialize the response into an `LlmResponse`. The built-in
 * `AnthropicProvider` and `OpenAiProvider` are the reference implementations.
 *
 * The returned `LlmResponse::message` SHALL use `Role::assistant` and SHALL contain the
 * model's text and/or tool-use blocks. Finish reason is a string — conventional values
 * are `"stop"`, `"tool_use"`, and `"length"` — and usage tokens should be filled in when
 * the backend returns them.
 *
 * Exceptions thrown from `chat()` are caught by the agent loop and re-thrown as
 * `AgentRunError` (kind `provider_error`) with the conversation preserved.
 *
 * ## Concurrency
 *
 * Concrete providers are not required to be thread-safe; the agent loop calls `chat()`
 * serially. If you share a provider across threads, synchronize externally.
 *
 * @see AnthropicProvider, OpenAiProvider, ChatRequest, LlmResponse, Agent::create
 */
class LlmProvider {
  public:
    virtual ~LlmProvider() = default;

    /**
     * @brief Send a conversation to the backend and return its next turn.
     *
     * @param conversation The running conversation including its (optional) system prompt.
     * @param request Optional tool definitions and output schema for this call.
     */
    virtual LlmResponse chat(const Conversation& conversation, const ChatRequest& request = {}) = 0;

    LlmProvider() = default;
    LlmProvider(const LlmProvider&) = delete;
    LlmProvider& operator=(const LlmProvider&) = delete;
    LlmProvider(LlmProvider&&) = default;
    LlmProvider& operator=(LlmProvider&&) = default;
};

} // namespace forgeant

#endif // FORGEANT_PROVIDER_PROVIDER_HPP
