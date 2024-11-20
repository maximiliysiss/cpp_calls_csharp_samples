# Benchmarks

## Description

This solution provides a set of ready-to-go benchmarks designed using the Google Benchmark library. It focuses on
delivering prebuilt performance tests for specific scenarios, algorithms, or workflows, enabling quick evaluation
without the need for custom setup or framework integration. These benchmarks are fully implemented and configured to
provide actionable insights into the performance of targeted code paths.

A unique feature of these benchmarks is their execution strategy: each test is run individually. This approach is
necessary because the .NET Runtime is loaded into the process only once. As a result, simultaneous execution of tests or
shared execution environments may lead to conflicts between the tests. By isolating each benchmark in its own execution,
we ensure reliable and accurate performance measurement while avoiding potential runtime clashes.

## Projects

The project is organized into two main components, each designed to streamline and ensure the accuracy of performance
testing:

1. `Benchmark` - This console application serves as the centralized hub for all defined performance tests. It is the
   core of the benchmarking suite, containing the implementation of all test cases for measuring and evaluating the
   performance of specific code sections or scenarios.
2. `Benchmark.Starter` - This console application is responsible for orchestrating the execution of the tests. It
   systematically runs each test one at a time, ensuring isolated execution to prevent interference between tests and to
   address scenarios where shared resources, such as the .NET Runtime, could otherwise cause conflicts.

## Table of results

| Benchmark                                                                      | Time      | CPU       | Iterations |
|--------------------------------------------------------------------------------|-----------|-----------|------------|
| BenchmarkFixture_call_aot/call_aot                                             | 4.04 ns   | 3.77 ns   | 186666667  |
| BenchmarkFixture_call_cppcli/call_cppcli                                       | 7.81 ns   | 7.85 ns   | 89600000   |
| BenchmarkFixture_call_framework_export/call_framework_export                   | 5.11 ns   | 5.02 ns   | 112000000  |
| BenchmarkFixture_call_standard_framework_export/call_standard_framework_export | 6.29 ns   | 6.14 ns   | 112000000  |
| call_com                                                                       | 141065 ns | 107422 ns | 6400       |
| BenchmarkFixture_call_old_runtime/call_old_runtime                             | 1257 ns   | 1245 ns   | 640000     |
| BenchmarkFixture_call_new_runtime/call_new_runtime                             | 146 ns    | 140 ns    | 5600000    |

## Summary of Benchmark Results

The benchmarks demonstrate the performance of various function call mechanisms, highlighting differences in execution
time and efficiency across different scenarios. Here's a summary:

1. Fastest Calls:
    + BenchmarkFixture_call_aot/call_aot: Achieved the best performance with a runtime of 4.04 ns, showcasing the
      efficiency of Ahead-of-Time (AOT) compilation.
    + BenchmarkFixture_call_framework_export/call_framework_export: Delivered a runtime of 5.11 ns, slightly slower but
      still highly efficient.

2. Moderately Fast Calls:
    + BenchmarkFixture_call_standard_framework_export/call_standard_framework_export: Executed in 6.29 ns, showing
      comparable performance to call_framework_export.
    + BenchmarkFixture_call_cppcli/call_cppcli: With a runtime of 7.81 ns, the C++/CLI call is efficient but slightly
      slower than AOT and framework export methods.

3. Slower Calls:
    + BenchmarkFixture_call_new_runtime/call_new_runtime: Performed in 146 ns, indicating that the newer runtime
      introduces additional overhead compared to AOT.
    + BenchmarkFixture_call_old_runtime/call_old_runtime: Executed in 1257 ns, revealing significant overhead in older
      runtime environments.

4. Slowest Call:
    + call_com: At 141065 ns, COM-based calls are the least efficient in this comparison, reflecting the additional
      overhead associated with the COM infrastructure.

Key Insights:

1. AOT and framework exports are the most efficient calling mechanisms, achieving nanosecond-level performance.
2. COM calls are significantly slower, likely due to their inherent complexity and inter-process communication overhead.
3. The new runtime demonstrates improved performance over the old runtime but still lags behind AOT in raw speed.

These benchmarks highlight the importance of choosing the right execution model based on performance requirements and
use cases.
