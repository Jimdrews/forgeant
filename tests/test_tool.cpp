#include <agentforge/tool/tool.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace agentforge;

struct AddParams {
    int a = 0;
    int b = 0;
};

template <>
struct agentforge::ParamSchema<AddParams> {
    static nlohmann::json schema() {
        return Schema::object()
            .property("a", Schema::integer().description("First number").build())
            .property("b", Schema::integer().description("Second number").build())
            .required({"a", "b"})
            .build();
    }
};

inline void from_json(const nlohmann::json& j, AddParams& p) {
    j.at("a").get_to(p.a);
    j.at("b").get_to(p.b);
}

TEST_CASE("make_tool creates tool with auto-generated schema", "[tool]") {
    auto tool =
        make_tool<AddParams>("add", "Add two numbers", [](AddParams p) { return p.a + p.b; });

    REQUIRE(tool.name == "add");
    REQUIRE(tool.description == "Add two numbers");
    REQUIRE(tool.parameters["type"] == "object");
    REQUIRE(tool.parameters["properties"].contains("a"));
    REQUIRE(tool.parameters["properties"].contains("b"));
}

TEST_CASE("make_tool execute deserializes and calls handler", "[tool]") {
    auto tool =
        make_tool<AddParams>("add", "Add two numbers", [](AddParams p) { return p.a + p.b; });

    auto result = tool.execute({{"a", 3}, {"b", 4}});
    REQUIRE(result == 7);
}

TEST_CASE("make_tool execute with string return", "[tool]") {
    auto tool = make_tool<AddParams>("add_str", "Add as string",
                                     [](AddParams p) { return std::to_string(p.a + p.b); });

    auto result = tool.execute({{"a", 3}, {"b", 4}});
    REQUIRE(result == "7");
}

TEST_CASE("Raw tool creation", "[tool]") {
    auto schema = Schema::object().property("x", Schema::integer().build()).build();

    Tool tool(
        "double_it", "Double a number", schema,
        [](const nlohmann::json& args) -> nlohmann::json { return args["x"].get<int>() * 2; });

    REQUIRE(tool.name == "double_it");
    auto result = tool.execute({{"x", 5}});
    REQUIRE(result == 10);
}

TEST_CASE("Tool execute throws on invalid args", "[tool]") {
    auto tool = make_tool<AddParams>("add", "Add", [](AddParams p) { return p.a + p.b; });

    REQUIRE_THROWS(tool.execute({{"invalid", "args"}}));
}
