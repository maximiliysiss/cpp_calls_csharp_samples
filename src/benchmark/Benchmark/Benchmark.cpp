#import "../../com/Com/bin/publish/Com.tlb" raw_interfaces_only

#include "Windows.h"
#include <filesystem>
#include <benchmark/benchmark.h>
#include <regex>

namespace fs = std::filesystem;

class BenchmarkFixture : public benchmark::Fixture
{
public:
    typedef int (*proc)(int, int);

protected:
    const char* name_;

    static HMODULE h_library_;
    static bool is_setup_done_;

public:
    BenchmarkFixture(const char* name)
    {
        name_ = name;
    }

    void SetUp(const ::benchmark::State& state) override
    {
        if (is_setup_done_)
            return;

        std::string library;

        for (const fs::path path("."); auto& p : fs::directory_iterator(path))
        {
            auto file_name = p.path().filename().string();

            if (std::regex_match(file_name, std::regex(name_)))
            {
                library = file_name;
                break;
            }
        }

        if (library.empty())
            throw std::runtime_error("Could not find any library");

        const std::wstring name(library.begin(), library.end());

        h_library_ = LoadLibrary(name.c_str());
        if (!h_library_)
            throw std::runtime_error("Failed to load library: " + library);

        is_setup_done_ = true;
    }

    proc get_function_pointer() const
    {
        if (!h_library_)
            throw std::runtime_error("Library is not loaded.");

        const auto function_name = "calculate";

        const auto func = (proc)GetProcAddress(h_library_, function_name);
        if (!func)
            throw std::runtime_error(std::string("Failed to get function: ") + function_name);

        return func;
    }
};

HMODULE BenchmarkFixture::h_library_ = nullptr;
bool BenchmarkFixture::is_setup_done_ = false;

#define RUN_TEST(method_name, pattern) \
class BenchmarkFixture_##method_name : public BenchmarkFixture { public: BenchmarkFixture_##method_name(): BenchmarkFixture(pattern) {}  };    \
BENCHMARK_F(BenchmarkFixture_##method_name, method_name)(benchmark::State& state) \
{ \
    const auto proc_address = get_function_pointer(); \
    for (auto _ : state) { \
        try \
        { \
            auto result = proc_address(1, 1); \
            benchmark::DoNotOptimize(result); \
        } \
        catch (const std::exception& e) \
        { \
            state.SkipWithError(e.what()); \
            break; \
        } \
    } \
}

std::string call_com()
{
    HRESULT hr = CoInitialize(nullptr); // Initialize COM
    if (FAILED(hr))
        return "Failed to initialize COM library.";

    try
    {
        // Create an instance of the COM class
        Com::IComInterfacePtr comObject;
        hr = comObject.CreateInstance(__uuidof(Com::ComInterface));
        if (FAILED(hr))
        {
            CoUninitialize();
            return "Failed to create COM interface.";
        }

        // Call the method
        long result = 0;
        comObject->Calculate(1, 1, &result);
    }
    catch (const _com_error& e)
    {
        return std::to_string(e.Error());
    }

    CoUninitialize(); // Clean up COM

    return "";
}

void call_com(benchmark::State& state)
{
    for (auto _ : state)
    {
        auto result = call_com();
        if (!result.empty())
        {
            state.SkipWithError(result);
            break;
        }
    }
}

// Register the function as a benchmark
RUN_TEST(call_aot, "^AheadOfTime_.*.dll$");
RUN_TEST(call_cppcli, "^CppCli.Wrapper.*.dll$");
RUN_TEST(call_framework_export, "^FrameworkExport_il.*.dll$");
RUN_TEST(call_standard_framework_export, "^StandardFrameworkExport.Wrapper_il.*.dll$");
RUN_TEST(call_old_runtime, "^OldRuntime.Wrapper.*.dll$");
RUN_TEST(call_new_runtime, "^NewRuntime.Wrapper.*.dll$");

BENCHMARK(call_com);

// Run the benchmark
BENCHMARK_MAIN();
