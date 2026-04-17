find_package(benchmark REQUIRED)

add_executable(agentforge_benchmarks
    bench_version.cpp
    bench_message.cpp
)

target_link_libraries(agentforge_benchmarks
    PRIVATE
        agentforge::agentforge
        benchmark::benchmark
        benchmark::benchmark_main
)
