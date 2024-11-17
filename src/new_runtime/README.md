# Using modern runtime

## Description

This solution shows the use of `hostfxr.dll` to bring up the .NET Runtime and use the .NET library.
Using `hostfxr.dll` is a new approach to bring up the .NET Runtime.

### Pros

1. No use of additional languages - only C++ and C#
2. Compatible with Unix/Windows

### Cons

1. Complex implementation
2. Requires defining additional wrapper C++ library

### Useful links

1. [coreclr_delegates.h](https://github.com/dotnet/runtime/blob/main/src/native/corehost/coreclr_delegates.h)
2. [hostfxr.h](https://github.com/dotnet/runtime/blob/main/src/native/corehost/hostfxr.h)
3. [Write a custom .NET host](https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting)
4. [.NET Sample](https://github.com/dotnet/samples/tree/main/core/hosting)

## Projects

The solution consists of 3 projects:

1. `NewRuntime.Api` - .NET library
2. `NewRuntime.Client` - client for the library
3. `NewRuntime.Wrapper` - C++ wrapper, which takes responsibility for loading .NET Runtime and loading .NET
   assemblies

### NewRuntime.Api

Standard library in .NET with API provided by `Api.cs`

```csharp
public static class Api
{
    // The method signature is below
    public delegate int CalculateDelegate(int a, int b);

    public static int Calculate(int a, int b) => a + b;
}
```

### NewRuntime.Wrapper

Wrapper is a C++ library that is responsible for raising .NET Runtime and loading .NET assembly. The project consists
of:

1. `Api.h/cpp` - API for `.dll`
2. `hostfxr.h, coreclr_delegates.h` - API `hostfxr.dll`
3. `Wrapper.h` - singleton (not thread safe) implementation for one-time raising of .NET Runtime and loading of assembly

#### Api.h/cpp

```c++
// Api.h
#define EXTERN_DLL_EXPORT extern "C" __declspec(dllexport)

EXTERN_DLL_EXPORT int calculate(int a, int b);

// Api.cpp
#include "Api.h"
#include "Wrapper.h"

int calculate(int a, int b)
{
    return Wrapper::get_instance(L"NewRuntime.Api", L"NewRuntime.Api.Api, NewRuntime.Api", L"Calculate",
                                 L"NewRuntime.Api.Api+CalculateDelegate, NewRuntime.Api")->calculate(a, b);
}
```

#### Wrapper.h

Represents a singleton class `Wrapper`, which defines the following methods:

1. `get_instance` - getting a single class object
2. `init` - raising .NET Runtime and loading the assembly
3. `calculate` - calling a method from the .NET library

The most interesting method is `init`, which loads .NET Runtime and the assembly

```c++
void init()
{
    // Step 1: Get hostfxr procedures
    const auto hostfxr_library = LoadLibraryW(L"hostfxr.dll");

    const auto init_proc = "hostfxr_initialize_for_runtime_config";
    const auto runtime_proc = "hostfxr_get_runtime_delegate";
    const auto close_proc = "hostfxr_close";

    const auto init_fptr = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(hostfxr_library, init_pr
    const auto get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)GetProcAddress(hostfxr_library, runtime_
    const auto close_fptr = (hostfxr_close_fn)GetProcAddress(hostfxr_library, close_proc);

    if (!(init_fptr && get_delegate_fptr && close_fptr))
        throw std::runtime_error("Failed to load hostfxr");

    const fs::path runtime_config_path = params_.assembly + L".runtimeconfig.json";

    // Step 2: Initialize the .NET Core runtime
    hostfxr_handle context = nullptr;

    int rc = init_fptr(runtime_config_path.c_str(), nullptr, &context);
    if (rc != 0 || context == nullptr)
        throw std::runtime_error("Failed to initialize runtime");

    // Step 3: Get the load_assembly_and_get_function_pointer delegate
    load_assembly_and_get_function_pointer_fn load_assembly_and_get_function_pointer = nullptr;

    rc = get_delegate_fptr(context,
                           hdt_load_assembly_and_get_function_pointer,
                           (void**)&load_assembly_and_get_function_pointer);
    if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
    {
        close_fptr(context);
        throw std::runtime_error("Failed to get load_assembly_and_get_function_pointer delegate");
    }

    // Step 4: Load the managed assembly and get a function pointer to the C# method
    rc = load_assembly_and_get_function_pointer(
        (params_.assembly + L".dll").c_str(), // assembly file
        params_.type.c_str(), // full qualified type
        params_.method.c_str(), // method
        params_.delegate.c_str(), // delegate full qualified type
        nullptr,
        (void**)&proc_);

    if (rc != 0 || proc_ == nullptr)
    {
        close_fptr(context);
        throw std::runtime_error("Failed to get managed method pointer");
    }
}
```

### NewRuntime.Client

A client for working with the wrapper library, which consists of one `.cpp` file.
The main method is called `call_library`, which is responsible for loading and calling the method from the wrapper
library

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
As a result, 3 files `NewRuntime.Api.dll`, `NewRuntime.Wrapper.dll`, `NewRuntime.Client.exe` will be obtained

## Executing files

If you call `dumpbin /exports` on the wrapper library, you can get the following output, where you can see that the
`Calculate` method we defined is available for export.

```dumpbin
Dump of file NewRuntime.Wrapper.dll

File Type: DLL

  Section contains the following exports for NewRuntime.Wrapper.dll

    00000000 characteristics
    FFFFFFFF time date stamp
        0.00 version
           1 ordinal base
           1 number of functions
           1 number of names

    ordinal hint RVA      name

          1    0 00011186 calculate = @ILT+385(calculate)

  Summary

        1000 .00cfg
        1000 .data
        2000 .idata
        1000 .msvcjmc
        3000 .pdata
        6000 .rdata
        1000 .reloc
        1000 .rsrc
       10000 .text
       10000 .textbss
```

If we call `dumpbin /exports` for the .NET library, we will not see a single export.

```dumpbin
Dump of file NewRuntime.Api.dll

File Type: DLL

  Summary

        2000 .reloc
        2000 .rsrc
        2000 .text
```

When running `.exe` you may get the following output:

```console
Success LoadLibrary("NewRuntime.Wrapper.dll")
Success GetProcAddress("calculate")
Calculation result is 2
```
