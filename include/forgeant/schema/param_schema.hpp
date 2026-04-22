#ifndef FORGEANT_SCHEMA_PARAM_SCHEMA_HPP
#define FORGEANT_SCHEMA_PARAM_SCHEMA_HPP

#include <forgeant/schema/schema.hpp>
#include <string>

namespace forgeant {

template <typename T>
struct ParamSchema {
    static_assert(sizeof(T) == 0, "ParamSchema not specialized for this type. "
                                  "Use FORGEANT_PARAMS or specialize ParamSchema<T>.");
};

template <>
struct ParamSchema<std::string> {
    static Json schema() { return Schema::string().build(); }
};

template <>
struct ParamSchema<int> {
    static Json schema() { return Schema::integer().build(); }
};

template <>
struct ParamSchema<double> {
    static Json schema() { return Schema::number().build(); }
};

template <>
struct ParamSchema<bool> {
    static Json schema() { return Schema::boolean().build(); }
};

} // namespace forgeant

#endif // FORGEANT_SCHEMA_PARAM_SCHEMA_HPP
