#ifndef FORGEANT_STRUCTURED_CONFIG_HPP
#define FORGEANT_STRUCTURED_CONFIG_HPP

namespace forgeant {

/**
 * @ingroup structured
 * @brief Retry policy for typed runs via `Agent::run<T>()` and the free `structured()` helper.
 *
 * When `Agent::run<T>()` is called with `T` other than `std::string`, the agent attaches
 * a JSON Schema derived from `ParamSchema<T>` to the request and parses the response
 * into `T`. If parsing fails (invalid JSON, missing required fields, type mismatch), the
 * agent appends a corrective user turn to the conversation ("Your response did not match
 * the required schema: …") and tries again, up to `max_retries` additional attempts.
 *
 * `max_retries` is the number of retries **after** the first attempt. With the default
 * of 2, the agent will try up to 3 times total before giving up.
 *
 * After the final attempt still fails, an `AgentRunError` with kind `structured_parse`
 * is thrown. The conversation on the error includes every failed attempt and every
 * corrective message, so you can inspect the full exchange.
 *
 * @see Agent::run, ParamSchema, AgentRunError, structured
 */
struct StructuredConfig {
    /// @brief Number of retries after the first attempt; total attempts is `max_retries + 1`.
    int max_retries = 2;
};

} // namespace forgeant

#endif // FORGEANT_STRUCTURED_CONFIG_HPP
