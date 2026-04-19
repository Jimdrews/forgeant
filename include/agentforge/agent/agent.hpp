#ifndef AGENTFORGE_AGENT_AGENT_HPP
#define AGENTFORGE_AGENT_AGENT_HPP

#include <agentforge/agent/config.hpp>
#include <agentforge/agent/options.hpp>
#include <agentforge/agent/result.hpp>
#include <agentforge/http/client.hpp>
#include <agentforge/provider/provider.hpp>
#include <agentforge/tool/registry.hpp>

#include <memory>
#include <string>

namespace agentforge {

class Agent {
  public:
    static std::unique_ptr<Agent> create(const std::string& provider, const AgentOptions& options);

    Agent(LlmProvider& provider, AgentConfig config = {});

    void add_tool(Tool tool);

    AgentResult run(const std::string& prompt);
    AgentResult chat(Conversation& conversation, const std::string& prompt);

  private:
    std::unique_ptr<HttpClient> owned_http_;
    std::unique_ptr<LlmProvider> owned_provider_;
    LlmProvider* provider_;
    ToolRegistry registry_;
    AgentConfig config_;

    Agent(std::unique_ptr<HttpClient> http, std::unique_ptr<LlmProvider> provider,
          AgentConfig config);

    AgentResult execute_loop(Conversation& conversation);
};

} // namespace agentforge

#endif // AGENTFORGE_AGENT_AGENT_HPP
