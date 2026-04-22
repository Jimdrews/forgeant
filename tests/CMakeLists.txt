find_package(Catch2 3 REQUIRED)
find_package(httplib REQUIRED)

add_executable(forgeant_tests
    test_json.cpp
    test_version.cpp
    test_role.cpp
    test_content.cpp
    test_message.cpp
    test_conversation.cpp
    test_http.cpp
    test_anthropic.cpp
    test_openai.cpp
    test_schema.cpp
    test_tool.cpp
    test_registry.cpp
    test_params_macro.cpp
    test_structured.cpp
    test_agent.cpp
    test_agent_factory.cpp
)

target_link_libraries(forgeant_tests
    PRIVATE
        forgeant::forgeant
        Catch2::Catch2WithMain
        httplib::httplib
)

include(Catch)
catch_discover_tests(forgeant_tests)
