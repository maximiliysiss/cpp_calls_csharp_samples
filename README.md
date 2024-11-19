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
| Old Runtime                             | Using pure .NET (Old) Runtime                    | .NET Framework, .NET Core, .NET Standard | [README.md](./src/old_runtime)               |
| .export instruction (Framework wrapper) | Using .export instruction                        | .NET Standard                            | [README.md](./src/standard_framework_export) |
