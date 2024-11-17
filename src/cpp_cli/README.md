# C++/CLI

## Description

C++/CLI is probably one of the most standard ways to implement interaction between C# and C++. In fact, the library on
C++/CLI is a router that takes responsibility for raising Runtime and calling managed code.

### Pros

1. Easy to implement
2. Lots of documentation
3. Takes care of some of the type marshalling issues
4. Supports all .NET Runtimes

### Cons

1. Requires C++ compiler for assembly
2. Additional code layer in the form of `.dll` library in C++
3. Requires .NET Runtime installed
4. Only for Windows machine

### Useful links

1. [.NET programming with C++/CLI](https://learn.microsoft.com/en-us/cpp/dotnet/dotnet-programming-with-cpp-cli-visual-cpp?view=msvc-170)
2. [C++/CLI](https://en.wikipedia.org/wiki/C%2B%2B/CLI)

## Projects

The example consists of 3 projects:

1. `CppCli.Api` - a library on .NET
2. `CppCli.Wrapper` - a library on C++/CLI aka wrapper
3. `CppCli.Client` - a client of the library

### CppCli.Api

Standard library in .NET with API provided by `Api.cs`

```csharp
public static class Api
{
    public static int Calculate(int a, int b) => a + b;
}
```

### CppCli.Wrapper

A C++ wrapper library that consists of 2 files to represent the native API

```c++
// Api.h
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

EXTERN_DLL_EXPORT int calculate(int a, int b);

// Api.cpp
#include "Api.h"

int calculate(int a, int b)
{
    return CppCli::Api::Api::Calculate(a, b);
}
```

### CppCli.Client

A client for working with the wrapper library, which consists of one `.cpp` file.
The main method is called `call_library`, which is responsible for loading and calling the method from the wrapper library

```c++
void call_library(const std::string& file_name)
{
    const std::wstring name(file_name.begin(), file_name.end());
    const auto library_handle = LoadLibrary(name.c_str());

    if (library_handle == nullptr)
    {
        std::cout << "Cannot open library by path '" << file_name << "'\n";
        return;
    }

    std::cout << "Success LoadLibrary(\"" << file_name << "\")\n";

    const auto proc_name = "calculate";
    const auto proc_address = (proc)GetProcAddress(library_handle, proc_name);

    if (proc_address == nullptr)
    {
        std::cout << "Cannot get proc '" << proc_name << """' in file '" << file_name << "'\n";
        return;
    }

    std::cout << "Success GetProcAddress(\"" << proc_name << "\")\n";

    std::cout << "Calculation result is " << proc_address(1, 1) << '\n';
}
```

## Assembly

The project is built from any IDE or console.
As a result, 3 files `CppCli.Api.dll`, `CppCli.Wrapper.dll`, `CppCli.Client.exe` will be obtained

## Executing files

If you call `dumpbin /exports` on the wrapper library, you can get the following output, where you can see that the
`Calculate` method we defined is available for export.

```dumpbin
Dump of file CppCli.Wrapper.dll

File Type: DLL

  Section contains the following exports for CppCli.Wrapper.dll

    00000000 characteristics
    FFFFFFFF time date stamp
        0.00 version
           1 ordinal base
           1 number of functions
           1 number of names

    ordinal hint RVA      name

          1    0 00005000 calculate = calculate

  Summary

        1000 .data
        1000 .nep
        1000 .pdata
        F000 .rdata
        1000 .reloc
        1000 .rsrc
        4000 .text
```

If we call `dumpbin /exports` for the .NET library, we will not see a single export.

```dumpbin
Dump of file CppCli.Api_net9.0.dll

File Type: DLL

  Summary

        2000 .reloc
        2000 .rsrc
        2000 .text
```

When running `.exe` you may get the following output:

```console
Success LoadLibrary("CppCli.Wrapper.dll")
Success GetProcAddress("calculate")
Calculation result is 2
```
