find_package(benchmark REQUIRED)
find_package(httplib REQUIRED)

add_executable(forgeant_benchmarks
    bench_version.cpp
    bench_message.cpp
    bench_http.cpp
    bench_provider.cpp
    bench_tool.cpp
    bench_structured.cpp
    bench_agent.cpp
)

target_link_libraries(forgeant_benchmarks
    PRIVATE
        forgeant::forgeant
        benchmark::benchmark
        benchmark::benchmark_main
        httplib::httplib
)
