add_executable(example_basic_chat basic_chat.cpp)
target_link_libraries(example_basic_chat PRIVATE forgeant::forgeant)

add_executable(example_tool_usage tool_usage.cpp)
target_link_libraries(example_tool_usage PRIVATE forgeant::forgeant)

add_executable(example_structured_output structured_output.cpp)
target_link_libraries(example_structured_output PRIVATE forgeant::forgeant)

add_executable(example_multi_turn multi_turn.cpp)
target_link_libraries(example_multi_turn PRIVATE forgeant::forgeant)
