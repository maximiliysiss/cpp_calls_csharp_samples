# Using old runtime

## Description

This solution shows the use of `coreclr.dll` to bring up the .NET Runtime and use the .NET library.
Using `coreclr.dll` is an old approach to bring up the .NET Runtime, the API of which can be found in
[coreclrhost.h](https://github.com/dotnet/runtime/blob/main/src/coreclr/hosts/inc/coreclrhost.h)

### Pros

1. No use of additional languages - only C++ and C#

### Cons

1. Complex implementation
2. Requires defining additional wrapper C++ library
3. Windows only
4. .NET Framework only

### Useful links

1. [coreclrhost.h](https://github.com/dotnet/runtime/blob/main/src/coreclr/hosts/inc/coreclrhost.h)

## Projects

The solution consists of 3 projects:

1. `OldRuntime.Api` - .NET library
2. `OldRuntime.Client` - client for the library
3. `OldRuntime.Wrapper` - C++ wrapper, which takes responsibility for loading .NET Runtime and loading .NET
   assemblies

### OldRuntime.Api

Standard library in .NET with API provided by `Api.cs`

```csharp
public static class Api
{
    public static int Calculate(int a, int b) => a + b;
}
```

### OldRuntime.Wrapper

Wrapper is a C++ library that is responsible for raising .NET Runtime and loading .NET assembly. The project consists
of:

1. `Api.h/cpp` - API for `.dll`
2. `coreclrhost.h` - API `coreclr.dll`
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
    return Wrapper::get_instance("OldRuntime.Api", "OldRuntime.Api.Api", "Calculate")->calculate(a, b);
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
   const auto executable_path = fs::current_path();
   const auto assembly_path = executable_path / (params_.assembly + ".dll;");

   // Step 1: Load coreclr.dll
   const auto coreclr_file_name = "coreclr.dll";
   const auto paths = split(getenv("PATH"), ';'); // Getting env 'PATH'
   const auto runtime_path = find_path(coreclr_file_name, paths); // .NET Runtime path

   const auto core_clr = LoadLibraryExA(coreclr_file_name, nullptr, 0);

   if (core_clr == nullptr)
       throw std::runtime_error("Failed to load CoreCLR");

   // Step 2: Get CoreCLR hosting functions
   const auto initialize_core_clr = (coreclr_initialize_ptr)GetProcAddress(core_clr, "coreclr_initialize");
   const auto create_managed_delegate = (coreclr_create_delegate_ptr)GetProcAddress(
       core_clr, "coreclr_create_delegate");
   const auto shutdown_core_clr = (coreclr_shutdown_ptr)GetProcAddress(core_clr, "coreclr_shutdown");

   if (!(initialize_core_clr && create_managed_delegate && shutdown_core_clr))
       throw std::runtime_error("Cannot get coreclr procedures");

   // Step 3: Construct AppDomain properties used when starting the runtime
   // Construct the trusted platform assemblies (TPA) list
   std::string tpa_list(assembly_path.string());
   for (const auto& file : fs::directory_iterator(runtime_path))
   {
       auto file_name = file.path().string();
       if (std::regex_match(file_name, std::regex(".*.dll$")))
           tpa_list += file_name + ';';
   }

   // Define CoreCLR properties
   const char* property_keys[] = {
       "TRUSTED_PLATFORM_ASSEMBLIES"
   };
   const char* property_values[] = {
       tpa_list.c_str(),
   };

   // Step 4: Start the CoreCLR runtime
   void* host_handle;
   unsigned int domain_id;
   int hr = initialize_core_clr(
       executable_path.string().c_str(), // AppDomain base path
       "SampleHost", // AppDomain friendly name
       sizeof(property_keys) / sizeof(char*), // Property count
       property_keys, // Property names
       property_values, // Property values
       &host_handle, // Host handle
       &domain_id); // AppDomain ID

   if (hr < 0)
       throw std::runtime_error("coreclr_initialize failed - status: " + std::to_string(hr));

   // Step 5: Create delegate to managed code
   hr = create_managed_delegate(
       host_handle,
       domain_id,
       params_.assembly.c_str(),
       params_.type.c_str(),
       params_.method.c_str(),
       (void**)&proc_);

   if (hr != S_OK)
   {
       shutdown_core_clr(host_handle, domain_id);
       throw std::runtime_error("coreclr_create_delegate failed - status: " + std::to_string(hr));
   }
}
```

### OldRuntime.Client

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
As a result, 3 files `OldRuntime.Api.dll`, `OldRuntime.Wrapper.dll`, `OldRuntime.Client.exe` will be obtained

## Executing files

If you call `dumpbin /exports` on the wrapper library, you can get the following output, where you can see that the
`Calculate` method we defined is available for export.

```dumpbin
Dump of file OldRuntime.Wrapper.dll

File Type: DLL

  Section contains the following exports for OldRuntime.Wrapper.dll

    00000000 characteristics
    FFFFFFFF time date stamp
        0.00 version
           1 ordinal base
           1 number of functions
           1 number of names

    ordinal hint RVA      name

          1    0 000486D6 calculate = @ILT+1745(calculate)

  Summary

        1000 .00cfg
        2000 .data
        3000 .idata
        1000 .msvcjmc
        A000 .pdata
       1A000 .rdata
        2000 .reloc
        1000 .rsrc
       9A000 .text
       47000 .textbss
```

If we call `dumpbin /exports` for the .NET library, we will not see a single export.

```dumpbin
Dump of file OldRuntime.Api.dll

File Type: DLL

  Summary

        2000 .reloc
        2000 .rsrc
        2000 .text
```

When running `.exe` you may get the following output:

```console
Success LoadLibrary("OldRuntime.Wrapper.dll")
Success GetProcAddress("calculate")
Calculation result is 2
```
