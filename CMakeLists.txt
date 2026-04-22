cmake_minimum_required(VERSION 3.21)

project(forgeant
    VERSION 0.2.0
    LANGUAGES CXX
    DESCRIPTION "C++ Agentic AI Framework"
)

find_package(glaze REQUIRED)
find_package(CURL REQUIRED)

add_library(forgeant
    src/json/json.cpp
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
add_library(forgeant::forgeant ALIAS forgeant)

target_compile_features(forgeant PUBLIC cxx_std_23)

target_include_directories(forgeant
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(forgeant
    PRIVATE
        glaze::glaze
        CURL::libcurl
)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(TARGETS forgeant
    EXPORT forgeantTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/forgeant
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT forgeantTargets
    FILE forgeantTargets.cmake
    NAMESPACE forgeant::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/forgeant
)

configure_package_config_file(
    cmake/forgeantConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/forgeantConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/forgeant
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/forgeantConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/forgeantConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/forgeantConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/forgeant
)

option(FORGEANT_BUILD_TESTS "Build tests" ${PROJECT_IS_TOP_LEVEL})
option(FORGEANT_BUILD_BENCHMARKS "Build benchmarks" ${PROJECT_IS_TOP_LEVEL})
option(FORGEANT_BUILD_EXAMPLES "Build examples" OFF)

if(FORGEANT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(FORGEANT_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(FORGEANT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
