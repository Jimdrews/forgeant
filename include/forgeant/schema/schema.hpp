#ifndef FORGEANT_SCHEMA_SCHEMA_HPP
#define FORGEANT_SCHEMA_SCHEMA_HPP

#include <forgeant/json/json.hpp>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>

namespace forgeant {

class SchemaBuilder {
  public:
    SchemaBuilder& description(std::string_view desc);
    SchemaBuilder& default_value(const Json& value);
    SchemaBuilder& enum_values(std::initializer_list<std::string> values);
    SchemaBuilder& property(const std::string& name, const Json& prop_schema);
    SchemaBuilder& required(std::initializer_list<std::string> fields);
    SchemaBuilder& items(const Json& item_schema);

    [[nodiscard]] Json build() const;

  private:
    friend class Schema;
    explicit SchemaBuilder(std::string type);

    Json schema_;
};

class Schema {
  public:
    static SchemaBuilder string();
    static SchemaBuilder integer();
    static SchemaBuilder number();
    static SchemaBuilder boolean();
    static SchemaBuilder object();
    static SchemaBuilder array();
};

} // namespace forgeant

#endif // FORGEANT_SCHEMA_SCHEMA_HPP
