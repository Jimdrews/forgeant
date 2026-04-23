#ifndef FORGEANT_SCHEMA_PARAM_SCHEMA_HPP
#define FORGEANT_SCHEMA_PARAM_SCHEMA_HPP

#include <forgeant/schema/schema.hpp>
#include <string>

namespace forgeant {

/**
 * @ingroup structured
 * @brief Compile-time mapping from a C++ type to its JSON Schema.
 *
 * `ParamSchema<T>::schema()` returns the JSON Schema that describes `T`. This powers
 * two features:
 *
 *   - `make_tool<Params>(...)` derives the tool's parameter schema from
 *     `ParamSchema<Params>::schema()`.
 *   - `Agent::run<T>()` (and the free `structured()` helper) derive the response schema
 *     from `ParamSchema<T>::schema()` to constrain the LLM's output.
 *
 * ## Built-in specializations
 *
 * Primitive types `std::string`, `int`, `double`, and `bool` are built in. Everything
 * else requires an explicit specialization.
 *
 * ## Extending to your own types
 *
 * Specialize `ParamSchema<MyType>` in the `forgeant` namespace and provide a static
 * `schema()` function returning `Json`. You will typically also need a `from_json`
 * overload so the parsed object can be constructed from the model's response:
 *
 * @code
 * struct MovieReview {
 *     std::string title;
 *     int rating;
 *     std::string summary;
 * };
 *
 * template <>
 * struct forgeant::ParamSchema<MovieReview> {
 *     static forgeant::Json schema() {
 *         return forgeant::Schema::object()
 *             .property("title",   forgeant::Schema::string().build())
 *             .property("rating",  forgeant::Schema::integer().build())
 *             .property("summary", forgeant::Schema::string().build())
 *             .required({"title", "rating", "summary"})
 *             .build();
 *     }
 * };
 *
 * void from_json(const forgeant::Json& j, MovieReview& out) {
 *     j.at("title").get_to(out.title);
 *     j.at("rating").get_to(out.rating);
 *     j.at("summary").get_to(out.summary);
 * }
 * @endcode
 *
 * The primary template is a `static_assert`-only placeholder: forgetting the
 * specialization produces a clear compile error rather than a mysterious link failure.
 *
 * @see Schema, SchemaBuilder, make_tool, Agent::run
 */
template <typename T>
struct ParamSchema {
    static_assert(sizeof(T) == 0, "ParamSchema not specialized for this type. "
                                  "Use FORGEANT_PARAMS or specialize ParamSchema<T>.");
};

/// @brief Built-in specialization: `std::string` maps to JSON Schema `"string"`.
template <>
struct ParamSchema<std::string> {
    static Json schema() { return Schema::string().build(); }
};

/// @brief Built-in specialization: `int` maps to JSON Schema `"integer"`.
template <>
struct ParamSchema<int> {
    static Json schema() { return Schema::integer().build(); }
};

/// @brief Built-in specialization: `double` maps to JSON Schema `"number"`.
template <>
struct ParamSchema<double> {
    static Json schema() { return Schema::number().build(); }
};

/// @brief Built-in specialization: `bool` maps to JSON Schema `"boolean"`.
template <>
struct ParamSchema<bool> {
    static Json schema() { return Schema::boolean().build(); }
};

} // namespace forgeant

#endif // FORGEANT_SCHEMA_PARAM_SCHEMA_HPP
