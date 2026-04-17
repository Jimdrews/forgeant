#ifndef AGENTFORGE_SCHEMA_PARAM_SCHEMA_HPP
#define AGENTFORGE_SCHEMA_PARAM_SCHEMA_HPP

#include <agentforge/schema/schema.hpp>

#include <string>

namespace agentforge {

template <typename T>
struct ParamSchema {
    static_assert(sizeof(T) == 0, "ParamSchema not specialized for this type. "
                                  "Use AGENTFORGE_PARAMS or specialize ParamSchema<T>.");
};

template <>
struct ParamSchema<std::string> {
    static nlohmann::json schema() { return Schema::string().build(); }
};

template <>
struct ParamSchema<int> {
    static nlohmann::json schema() { return Schema::integer().build(); }
};

template <>
struct ParamSchema<double> {
    static nlohmann::json schema() { return Schema::number().build(); }
};

template <>
struct ParamSchema<bool> {
    static nlohmann::json schema() { return Schema::boolean().build(); }
};

} // namespace agentforge

#endif // AGENTFORGE_SCHEMA_PARAM_SCHEMA_HPP
