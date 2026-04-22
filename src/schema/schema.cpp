#include <forgeant/schema/schema.hpp>

namespace forgeant {

SchemaBuilder::SchemaBuilder(std::string type) {
    schema_["type"] = std::move(type);
}

SchemaBuilder& SchemaBuilder::description(std::string_view desc) {
    schema_["description"] = std::string(desc);
    return *this;
}

SchemaBuilder& SchemaBuilder::default_value(const Json& value) {
    schema_["default"] = value;
    return *this;
}

SchemaBuilder& SchemaBuilder::enum_values(std::initializer_list<std::string> values) {
    schema_["enum"] = Json::array();
    for (const auto& v : values) {
        schema_["enum"].push_back(v);
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::property(const std::string& name, const Json& prop_schema) {
    if (!schema_.contains("properties")) {
        schema_["properties"] = Json::object();
    }
    schema_["properties"][name] = prop_schema;
    return *this;
}

SchemaBuilder& SchemaBuilder::required(std::initializer_list<std::string> fields) {
    schema_["required"] = Json::array();
    for (const auto& f : fields) {
        schema_["required"].push_back(f);
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::items(const Json& item_schema) {
    schema_["items"] = item_schema;
    return *this;
}

Json SchemaBuilder::build() const {
    return schema_;
}

SchemaBuilder Schema::string() {
    return SchemaBuilder("string");
}
SchemaBuilder Schema::integer() {
    return SchemaBuilder("integer");
}
SchemaBuilder Schema::number() {
    return SchemaBuilder("number");
}
SchemaBuilder Schema::boolean() {
    return SchemaBuilder("boolean");
}
SchemaBuilder Schema::object() {
    return SchemaBuilder("object");
}
SchemaBuilder Schema::array() {
    return SchemaBuilder("array");
}

} // namespace forgeant
