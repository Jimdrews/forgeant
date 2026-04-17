find_package(Catch2 3 REQUIRED)

add_executable(agentforge_tests
    test_version.cpp
    test_role.cpp
    test_content.cpp
    test_message.cpp
    test_conversation.cpp
)

target_link_libraries(agentforge_tests
    PRIVATE
        agentforge::agentforge
        Catch2::Catch2WithMain
)

include(Catch)
catch_discover_tests(agentforge_tests)
