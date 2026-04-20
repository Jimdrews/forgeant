cmake_minimum_required(VERSION 3.21)

project(agentforge
    VERSION 0.2.0
    LANGUAGES CXX
    DESCRIPTION "C++ Agentic AI Framework"
)

find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

add_library(agentforge
    src/version.cpp
    src/types/role.cpp
    src/types/content.cpp
    src/types/message.cpp
    src/types/conversation.cpp
    src/http/curl_client.cpp
    src/provider/anthropic.cpp
    src/provider/openai.cpp
    src/schema/schema.cpp
    src/tool/registry.cpp
    src/structured/structured.cpp
    src/agent/agent.cpp
    src/agent/error.cpp
)
add_library(agentforge::agentforge ALIAS agentforge)

target_compile_features(agentforge PUBLIC cxx_std_20)

target_include_directories(agentforge
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(agentforge
    PUBLIC
        nlohmann_json::nlohmann_json
    PRIVATE
        CURL::libcurl
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS agentforge
    EXPORT agentforgeTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/agentforge
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT agentforgeTargets
    FILE agentforgeTargets.cmake
    NAMESPACE agentforge::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/agentforge
)

configure_package_config_file(
    cmake/agentforgeConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/agentforgeConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/agentforge
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/agentforgeConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/agentforgeConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/agentforgeConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/agentforge
)

option(AGENTFORGE_BUILD_TESTS "Build tests" ${PROJECT_IS_TOP_LEVEL})
option(AGENTFORGE_BUILD_BENCHMARKS "Build benchmarks" ${PROJECT_IS_TOP_LEVEL})
option(AGENTFORGE_BUILD_EXAMPLES "Build examples" OFF)

if(AGENTFORGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(AGENTFORGE_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(AGENTFORGE_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
