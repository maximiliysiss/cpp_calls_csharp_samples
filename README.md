# cpp_calls_csharp_samples

## Description

Samples of calling C# from C++ code

## Implementations

| Name                                    | Description                                      | Target runtime                           | Readme                                       |
|-----------------------------------------|--------------------------------------------------|------------------------------------------|----------------------------------------------|
| AOT                                     | Using ahead of time compilation into native .dll | .NET 8, .NET 9                           | [README.md](./src/aot)                       |
| COM                                     | Using COM Interop                                | .NET Framework, .NET Core, .NET Standard | [README.md](./src/com)                       |
| C++/CLI                                 | Using C++/CLI like Wrapper .dll                  | .NET Framework, .NET Core, .NET Standard | [README.md](./src/cpp_cli)                   |
| .export instruction                     | Using .export instruction                        | .NET Framework                           | [README.md](./src/framework_export)          |
| New Runtime                             | Using pure .NET (New) Runtime                    | .NET Standard, .NET Core                 | [README.md](./src/new_runtime)               |
| New Runtime (delegate free)             | Using pure .NET (New) Runtime                    | .NET Standard, .NET Core                 | [README.md](./src/new_runtime_delegate_free) |
| Old Runtime                             | Using pure .NET (Old) Runtime                    | .NET Framework, .NET Core, .NET Standard | [README.md](./src/old_runtime)               |
| .export instruction (Framework wrapper) | Using .export instruction                        | .NET Standard                            | [README.md](./src/standard_framework_export) |

## [Benchmarks](./src/benchmark)

| Benchmark                                                                      | Time      | CPU       | Iterations |
|--------------------------------------------------------------------------------|-----------|-----------|------------|
| BenchmarkFixture_call_aot/call_aot                                             | 4.04 ns   | 3.77 ns   | 186666667  |
| BenchmarkFixture_call_cppcli/call_cppcli                                       | 7.81 ns   | 7.85 ns   | 89600000   |
| BenchmarkFixture_call_framework_export/call_framework_export                   | 5.11 ns   | 5.02 ns   | 112000000  |
| BenchmarkFixture_call_standard_framework_export/call_standard_framework_export | 6.29 ns   | 6.14 ns   | 112000000  |
| call_com                                                                       | 141065 ns | 107422 ns | 6400       |
| BenchmarkFixture_call_old_runtime/call_old_runtime                             | 1257 ns   | 1245 ns   | 640000     |
| BenchmarkFixture_call_new_runtime/call_new_runtime                             | 146 ns    | 140 ns    | 5600000    |
| BenchmarkFixture_call_free_delegate_new_runtime/call_free_delegate_new_runtime | 113 ns    | 107 ns    | 6400000    |
