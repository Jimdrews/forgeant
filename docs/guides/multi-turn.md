# Multi-turn conversations {#multi-turn-guide}

`Agent::run(prompt)` starts a fresh conversation each time. For multi-turn exchanges — where the model needs to remember what was said before — pass a `forgeant::Conversation` instead.

## The pattern

Every `forgeant::AgentResult` carries a `conversation` field: the full transcript including the user's input, the assistant's reply, and any tool-use/tool-result blocks along the way. Feed that conversation back into the next `run()` to continue:

```cpp
auto agent = forgeant::Agent::create("anthropic", {
    .api_key = std::getenv("ANTHROPIC_API_KEY"),
    .model = "claude-sonnet-4-20250514",
    .system_prompt = "You are a helpful assistant.",
});

// Turn 1
auto r1 = agent->run("My name is Alice. Remember that.");

// Turn 2 — carry the conversation forward
forgeant::Conversation conv = r1.conversation;
conv.add(forgeant::Message(forgeant::Role::user, "What is my name?"));
auto r2 = agent->run(conv);
// r2.output → "Your name is Alice."

// Turn 3
conv = r2.conversation;
conv.add(forgeant::Message(forgeant::Role::user,
                           "How many messages have we exchanged?"));
auto r3 = agent->run(conv);
```

Each call copies the conversation internally, so the caller's copy is never mutated. The returned `AgentResult::conversation` is always the fresh transcript.

## Building a `Conversation` from scratch

You don't have to start from a previous run — you can construct a `Conversation` directly:

```cpp
forgeant::Conversation conv("You are a terse assistant.");
conv.add(forgeant::Message(forgeant::Role::user, "Hello."));
conv.add(forgeant::Message(forgeant::Role::assistant, "Hi."));
conv.add(forgeant::Message(forgeant::Role::user, "What did I just say?"));

auto result = agent->run(conv);
```

The system prompt on the conversation takes precedence over `AgentOptions::system_prompt` (but is itself overridden by `forgeant::RunOverrides::system_prompt` for a single call).

## Inspecting the transcript

`Conversation::messages()` returns a `std::span<const Message>`. Each `forgeant::Message` has a `role` and a vector of `forgeant::ContentBlock` — the content blocks are one of `TextBlock`, `ToolUseBlock`, or `ToolResultBlock`. Use `std::visit` to handle each type:

```cpp
for (const auto& msg : result.conversation.messages()) {
    for (const auto& block : msg.content) {
        std::visit([](const auto& b) {
            using T = std::decay_t<decltype(b)>;
            if constexpr (std::is_same_v<T, forgeant::TextBlock>) {
                std::cout << b.text << "\n";
            } else if constexpr (std::is_same_v<T, forgeant::ToolUseBlock>) {
                std::cout << "[tool call: " << b.name << "]\n";
            } else if constexpr (std::is_same_v<T, forgeant::ToolResultBlock>) {
                std::cout << "[tool result for " << b.tool_use_id << "]\n";
            }
        }, block);
    }
}
```

## Where to look next

- @ref types — `Conversation`, `Message`, `Role`, and the `ContentBlock` variants.
- @ref agents — `AgentResult` shape and the relationship between calls.

@example multi_turn.cpp
