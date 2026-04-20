#include <agentforge/tool/registry.hpp>

#include <catch2/catch_test_macros.hpp>

using namespace agentforge;

struct EchoParams {
    std::string text;
};

template <>
struct agentforge::ParamSchema<EchoParams> {
    static nlohmann::json schema() {
        return Schema::object()
            .property("text", Schema::string().build())
            .required({"text"})
            .build();
    }
};

inline void from_json(const nlohmann::json& j, EchoParams& p) {
    j.at("text").get_to(p.text);
}

TEST_CASE("ToolRegistry add and get", "[registry]") {
    ToolRegistry registry;
    registry.add(make_tool<EchoParams>("echo", "Echo text", [](EchoParams p) { return p.text; }));

    REQUIRE(registry.has("echo"));
    REQUIRE(registry.get("echo").name == "echo");
    REQUIRE(registry.size() == 1);
}

TEST_CASE("ToolRegistry unknown tool throws", "[registry]") {
    ToolRegistry registry;
    REQUIRE_THROWS_AS(registry.get("nonexistent"), std::runtime_error);
}

TEST_CASE("ToolRegistry execute dispatches by name", "[registry]") {
    ToolRegistry registry;
    registry.add(make_tool<EchoParams>("echo", "Echo text", [](EchoParams p) { return p.text; }));

    auto result = registry.execute("echo", {{"text", "hello"}});
    REQUIRE(result == "hello");
}

TEST_CASE("ToolRegistry tools returns neutral views", "[registry]") {
    ToolRegistry registry;
    registry.add(make_tool<EchoParams>("echo", "Echo text", [](EchoParams p) { return p.text; }));

    auto views = registry.tools();
    REQUIRE(views.size() == 1);
    REQUIRE(views[0].name == "echo");
    REQUIRE(views[0].description == "Echo text");
    REQUIRE(views[0].parameters.at("type") == "object");
}

TEST_CASE("ToolRegistry tools views borrow from registry storage", "[registry]") {
    ToolRegistry registry;
    registry.add(make_tool<EchoParams>("echo", "Echo text", [](EchoParams p) { return p.text; }));

    auto views = registry.tools();
    REQUIRE(views.size() == 1);
    const auto& stored = registry.get("echo");
    REQUIRE(views[0].name.data() == stored.name.data());
    REQUIRE(views[0].description.data() == stored.description.data());
    REQUIRE(&views[0].parameters == &stored.parameters);
}

TEST_CASE("ToolRegistry multi-tool dispatch", "[registry]") {
    ToolRegistry registry;
    registry.add(make_tool<EchoParams>("echo", "Echo", [](EchoParams p) { return p.text; }));
    registry.add(Tool("upper", "Uppercase",
                      Schema::object().property("text", Schema::string().build()).build(),
                      [](const nlohmann::json& args) -> nlohmann::json {
                          auto text = args["text"].get<std::string>();
                          std::transform(text.begin(), text.end(), text.begin(), ::toupper);
                          return text;
                      }));

    auto r1 = registry.execute("echo", {{"text", "hello"}});
    auto r2 = registry.execute("upper", {{"text", "hello"}});
    REQUIRE(r1 == "hello");
    REQUIRE(r2 == "HELLO");
}
