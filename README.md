# cpp_calls_csharp_samples

## Description

Samples of calling C# from C++ code

## Implementations

| Name                                    | Description                                      | Target runtime                           | Readme                                                 |
|-----------------------------------------|--------------------------------------------------|------------------------------------------|--------------------------------------------------------|
| AOT                                     | Using ahead of time compilation into native .dll | .NET 8, .NET 9                           | [README.md](./src/aot/README.md)                       |
| COM                                     | Using COM Interop                                | .NET Framework, .NET Core                | [README.md](./src/com/README.md)                       |
| C++/CLI                                 | Using C++/CLI like Wrapper .dll                  | .NET Framework, .NET Core, .NET Standard | [README.md](./src/cpp_cli/README.md)                   |
| .export instruction                     | Using .export instruction                        | .NET Framework                           | [README.md](./src/framework_export/README.md)          |
| New Runtime                             | Using pure .NET (New) Runtime                    | .NET Standard, .NET Core                 | [README.md](./src/new_runtime/README.md)               |
| Old Runtime                             | Using pure .NET (Old) Runtime                    | .NET Framework                           | [README.md](./src/old_runtime/README.md)               |
| .export instruction (Framework wrapper) | Using .export instruction                        | .NET Standard                            | [README.md](./src/standard_framework_export/README.md) |
