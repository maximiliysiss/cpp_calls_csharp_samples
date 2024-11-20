#include <iostream>
#include <vector>

static void run_benchmark(const std::string& benchmark_name)
{
    const auto command = "Benchmark.exe --benchmark_filter=" + benchmark_name;

    if (system(command.c_str()) != 0)
        std::cerr << "Failed to run benchmark: " << benchmark_name << std::endl;
}

int main()
{
    const std::vector<std::string> benchmarks = {
        "call_aot",
        "call_cppcli",
        "call_framework_export",
        "call_standard_framework_export",
        "call_com",
        "call_old_runtime",
        "call_new_runtime",
    };

    for (const auto& benchmark : benchmarks)
        run_benchmark(benchmark);

    return 0;
}
