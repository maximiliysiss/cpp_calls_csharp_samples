# Using modern runtime without delegate declaration

## Description

A similar approach is used in the `new_runtime` project. For a detailed explanation of the implementation, refer to the
[README](../new_runtime/README.md).

The key difference lies in the absence of a delegate definition in Api.cs. However, to ensure compatibility when
invoking the `load_assembly_and_get_function_pointer` function, it is necessary to include the `UnmanagedCallersOnly`
attribute. This attribute simplifies direct interaction with unmanaged code by specifying that the method can be called
from native code without additional interop layers.

### Useful links

1. [coreclr_delegates.h](https://github.com/dotnet/runtime/blob/main/src/native/corehost/coreclr_delegates.h)
2. [hostfxr.h](https://github.com/dotnet/runtime/blob/main/src/native/corehost/hostfxr.h)
3. [Write a custom .NET host](https://learn.microsoft.com/en-us/dotnet/core/tutorials/netcore-hosting)
4. [.NET Sample](https://github.com/dotnet/samples/tree/main/core/hosting)

## Projects

The solution consists of 3 projects:

1. `NewRuntimeDelegateFree.Api` - .NET library
2. `NewRuntimeDelegateFree.Client` - client for the library
3. `NewRuntimeDelegateFree.Wrapper` - C++ wrapper, which takes responsibility for loading .NET Runtime and loading .NET
   assemblies

### NewRuntimeDelegateFree.Api

Standard library in .NET with API provided by `Api.cs`

```csharp
public static class Api
{
    [UnmanagedCallersOnly]
    public static int Calculate(int a, int b) => a + b;
}
```

### NewRuntimeDelegateFree.Wrapper

Wrapper is a C++ library that is responsible for raising .NET Runtime and loading .NET assembly. The project consists
of:

1. `Api.h/cpp` - API for `.dll`
2. `hostfxr.h, coreclr_delegates.h` - API `hostfxr.dll`
3. `Wrapper.h` - singleton (not thread safe) implementation for one-time raising of .NET Runtime and loading of assembly

The only notable difference compared to the code from [new_runtime](../new_runtime/NewRuntime.Wrapper/Wrapper.h) lies in
the invocation of the load_assembly_and_get_function_pointer function.

In this project, the parameter for `delegate_type_name` is set to `UNMANAGEDCALLERSONLY_METHOD` instead of the
delegate's name. This adjustment leverages the `UnmanagedCallersOnly` attribute, which eliminates the need for a
delegate type, allowing for a more direct and efficient interaction with unmanaged code. This approach simplifies the
process while maintaining compatibility with the requirements of `load_assembly_and_get_function_pointer`.

```c++
rc = load_assembly_and_get_function_pointer(
    (params_.assembly + L".dll").c_str(), // assembly file
    params_.type.c_str(), // full qualified type
    params_.method.c_str(), // method
    UNMANAGEDCALLERSONLY_METHOD,
    nullptr,
    (void**)&proc_);
```

### NewRuntimeDelegateFree.Client

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

## Executing files

If you call `dumpbin /exports` on the wrapper library, you can get the following output, where you can see that the
`Calculate` method we defined is available for export.

```dumpbin
Dump of file Wrapper.dll

File Type: DLL

  Section contains the following exports for Wrapper.dll

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
Dump of file Api.dll

File Type: DLL

  Summary

        2000 .reloc
        2000 .rsrc
        2000 .text
```

When running `.exe` you may get the following output:

```console
Success LoadLibrary("NewRuntimeDelegateFree.Wrapper.dll")
Success GetProcAddress("calculate")
Calculation result is 2
```
