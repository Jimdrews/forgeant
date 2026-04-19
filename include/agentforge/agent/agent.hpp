#ifndef AGENTFORGE_AGENT_AGENT_HPP
#define AGENTFORGE_AGENT_AGENT_HPP

#include <agentforge/agent/config.hpp>
#include <agentforge/agent/result.hpp>
#include <agentforge/provider/provider.hpp>
#include <agentforge/tool/registry.hpp>

#include <string>

namespace agentforge {

class Agent {
  public:
    Agent(LlmProvider& provider, AgentConfig config = {});

    void add_tool(Tool tool);

    AgentResult run(const std::string& prompt);
    AgentResult chat(Conversation& conversation, const std::string& prompt);

  private:
    LlmProvider& provider_;
    ToolRegistry registry_;
    AgentConfig config_;

    AgentResult execute_loop(Conversation& conversation);
};

} // namespace agentforge

#endif // AGENTFORGE_AGENT_AGENT_HPP
