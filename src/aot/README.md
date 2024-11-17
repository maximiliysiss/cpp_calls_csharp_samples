# Ahead of time

## Description

Sample of calling C# from C++ code where
using [Ahead of time](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/?tabs=windows%2Cnet8).
This is the most modern and simple way to present a .NET library as native compatible.

### Pros

1. Simple implementation: add attribute, configure `.csproj`
2. Supported by modern Runtimes (.NET 8, .NET 9)

### Cons

1. Imposes restrictions: you cannot use dynamics - for example, `Assembly.Load`, `Reflection`
2. Not supported by older Runtimes (.NET Framework)

### Useful links

1. [Native AOT deployment](https://learn.microsoft.com/en-us/dotnet/core/deploying/native-aot/?tabs=windows%2Cnet8)
2. [.NET Sample](https://github.com/dotnet/samples/tree/main/core/nativeaot)

## Projects

The solution consists of two projects: a library and a client.

### AheadOfTime

A library project that supports AOT assembly.
Consists of 1 file `Api.cs`, which contains a method for calling from outside.

```csharp
public static class Api
{
    [UnmanagedCallersOnly(EntryPoint = "Calculate")]
    public static int Calculate(int a, int b) => a + b;
}
```

The project is configured to build in AOT

```xml

<Project Sdk="Microsoft.NET.Sdk">
    <PropertyGroup>
        <PublishAot>true</PublishAot>
        <IsAotCompatible>true</IsAotCompatible>
        <AssemblyName>AheadOfTime_$(TargetFramework)_$(RuntimeIdentifier)</AssemblyName>
    </PropertyGroup>
</Project>
```

To get a native build, you need to call the command `dotnet publish -f net8.0 -r win-x64`

### AheadOfTime.Client

A project that provides a simple client for opening a library via `LoadLibrary` and calling a method obtained from
`GetProcAddress`.
Consists of a single `.cpp` file, the main one being the `call_library` method, which describes the logic for calling a
method from the
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

    const auto proc_name = "Calculate";
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
As a result, 2 files `AheadOfTime_.._...dll` and `AheadOfTime.Client.exe` will be obtained

## Executing files

If you call `dumpbin /exports` on the library, you can get the following output, where you can see that the `Calculate`
method we defined is available for export.

```dumpbin
Dump of file AheadOfTime_net8.0_win-x64.dll

File Type: DLL

  Section contains the following exports for AheadOfTime_net8.0_win-x64.dll

    00000000 characteristics
    FFFFFFFF time date stamp
        0.00 version
           1 ordinal base
           2 number of functions
           2 number of names

    ordinal hint RVA      name

          1    0 00071BD0 Calculate = AheadOfTime_net8_0_win_x64_AheadOfTime_Api__Calculate
          2    1 001756B0 DotNetRuntimeDebugHeader = DotNetRuntimeDebugHeader

  Summary

        E000 .data
       68000 .managed
        C000 .pdata
       6A000 .rdata
        1000 .reloc
        1000 .rsrc
       70000 .text
       32000 hydrated
```

When running `.exe` you may get the following output:

```console
Success LoadLibrary("AheadOfTime_net8.0_win-x64.dll")
Success GetProcAddress("Calculate")
Calculation result is 2

Success LoadLibrary("AheadOfTime_net9.0_win-x64.dll")
Success GetProcAddress("Calculate")
Calculation result is 2
```
