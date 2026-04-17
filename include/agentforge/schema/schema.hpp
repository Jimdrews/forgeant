#ifndef AGENTFORGE_SCHEMA_SCHEMA_HPP
#define AGENTFORGE_SCHEMA_SCHEMA_HPP

#include <initializer_list>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace agentforge {

class SchemaBuilder {
  public:
    SchemaBuilder& description(std::string_view desc);
    SchemaBuilder& default_value(const nlohmann::json& value);
    SchemaBuilder& enum_values(std::initializer_list<std::string> values);
    SchemaBuilder& property(const std::string& name, const nlohmann::json& prop_schema);
    SchemaBuilder& required(std::initializer_list<std::string> fields);
    SchemaBuilder& items(const nlohmann::json& item_schema);

    [[nodiscard]] nlohmann::json build() const;

  private:
    friend class Schema;
    explicit SchemaBuilder(std::string type);

    nlohmann::json schema_;
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

} // namespace agentforge

#endif // AGENTFORGE_SCHEMA_SCHEMA_HPP
