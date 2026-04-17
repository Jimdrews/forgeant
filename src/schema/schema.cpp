#include <agentforge/schema/schema.hpp>

namespace agentforge {

SchemaBuilder::SchemaBuilder(std::string type) {
    schema_["type"] = std::move(type);
}

SchemaBuilder& SchemaBuilder::description(std::string_view desc) {
    schema_["description"] = std::string(desc);
    return *this;
}

SchemaBuilder& SchemaBuilder::default_value(const nlohmann::json& value) {
    schema_["default"] = value;
    return *this;
}

SchemaBuilder& SchemaBuilder::enum_values(std::initializer_list<std::string> values) {
    schema_["enum"] = nlohmann::json::array();
    for (const auto& v : values) {
        schema_["enum"].push_back(v);
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::property(const std::string& name, const nlohmann::json& prop_schema) {
    if (!schema_.contains("properties")) {
        schema_["properties"] = nlohmann::json::object();
    }
    schema_["properties"][name] = prop_schema;
    return *this;
}

SchemaBuilder& SchemaBuilder::required(std::initializer_list<std::string> fields) {
    schema_["required"] = nlohmann::json::array();
    for (const auto& f : fields) {
        schema_["required"].push_back(f);
    }
    return *this;
}

SchemaBuilder& SchemaBuilder::items(const nlohmann::json& item_schema) {
    schema_["items"] = item_schema;
    return *this;
}

nlohmann::json SchemaBuilder::build() const {
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

} // namespace agentforge
