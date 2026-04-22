#ifndef FORGEANT_AGENT_RUN_OVERRIDES_HPP
#define FORGEANT_AGENT_RUN_OVERRIDES_HPP

#include <optional>
#include <string>

namespace forgeant {

struct RunOverrides {
    std::optional<std::string> system_prompt;
    std::optional<int> max_iterations;
};

} // namespace forgeant

#endif // FORGEANT_AGENT_RUN_OVERRIDES_HPP
