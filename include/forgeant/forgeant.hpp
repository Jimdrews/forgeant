#ifndef FORGEANT_FORGEANT_HPP
#define FORGEANT_FORGEANT_HPP

/**
 * @defgroup agents Agents
 * @brief The agent runtime: configuration, the ReAct-style loop, and its results and errors.
 *
 * @defgroup tools Tools
 * @brief Defining callable tools and making them available to the agent.
 *
 * @defgroup providers Providers
 * @brief LLM backend interface and the built-in Anthropic, OpenAI, and Ollama implementations.
 *
 * @defgroup structured Structured Output
 * @brief Typed, schema-validated responses via `run<T>()`, including JSON Schema construction.
 *
 * @defgroup types Core Types
 * @brief Messages, roles, content blocks, conversations, and usage accounting.
 *
 * @defgroup transport Transport
 * @brief HTTP client abstraction and the curl-backed default implementation.
 */

#include <forgeant/agent/agent.hpp>
#include <forgeant/agent/error.hpp>
#include <forgeant/agent/options.hpp>
#include <forgeant/agent/result.hpp>
#include <forgeant/agent/run_overrides.hpp>
#include <forgeant/http/client.hpp>
#include <forgeant/http/curl_client.hpp>
#include <forgeant/http/response.hpp>
#include <forgeant/provider/anthropic.hpp>
#include <forgeant/provider/chat_request.hpp>
#include <forgeant/provider/config.hpp>
#include <forgeant/provider/openai.hpp>
#include <forgeant/provider/provider.hpp>
#include <forgeant/provider/response.hpp>
#include <forgeant/schema/param_schema.hpp>
#include <forgeant/schema/schema.hpp>
#include <forgeant/structured/config.hpp>
#include <forgeant/structured/structured.hpp>
#include <forgeant/tool/registry.hpp>
#include <forgeant/tool/tool.hpp>
#include <forgeant/tool/tool_view.hpp>
#include <forgeant/types.hpp>
#include <forgeant/version.hpp>

#endif // FORGEANT_FORGEANT_HPP
