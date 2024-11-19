# COM

## Description

The Component Object Model (COM) lets an object expose its functionality to other components and to host applications on
Windows platforms. To help enable users to interoperate with their existing code bases, .NET Framework has always
provided strong support for interoperating with COM libraries.

### Pros

1. Standardized implementation for interoperability between C++ and C#
2. There is a lot of documentation
3. Supports all .NET Runtimes

### Cons

1. Requires the client to support interaction via COM
2. Requires writing specific code and special actions for deployment
3. Only for Windows machine

### Useful links

1. [COM Interop in .NET](https://learn.microsoft.com/en-us/dotnet/standard/native-interop/cominterop)

## Projects

The solution contains 2 projects:

1. `Com` - a library that is automatically registered after assembly
2. `Com.Client` - a client that supports working with the COM library

### Com

COM component library project.
Consists of a COM interface and its implementation.

```csharp
[ComVisible(true)]
[InterfaceType(ComInterfaceType.InterfaceIsDual)]
public interface IComInterface
{
    int Calculate(int a, int b);
}

[ComVisible(true)]
[ClassInterface(ClassInterfaceType.None)]
public sealed class ComInterface : IComInterface
{
    public int Calculate(int a, int b) => a + b;
}
```

To build a project, you need to define several additional attributes:

```xml

<PropertyGroup>
    <ComVisible>true</ComVisible>
</PropertyGroup>
```

Also added `Targets` after build to deploy a COM component which consists of:

1. Registering a COM component and generating a `.tlb` file that will be used as `#import` in a C++ project
2. Copying files for import to the `publish` folder
3. Registering the `.dll` file in the [GAC](https://learn.microsoft.com/en-us/dotnet/framework/app-domains/gac)

```xml

<Target Name="ComRegister" AfterTargets="Build">
    <Message Text="Register COM component" Importance="high"/>
    <Exec
        Command="C:\Windows\Microsoft.NET\Framework64\v4.0.30319\regasm $(TargetDir)$(AssemblyName).dll /codebase /tlb:$(AssemblyName).tlb"/>

    <Message Text="Publish TLB" Importance="high"/>
    <Copy SourceFiles="$(TargetDir)$(AssemblyName).tlb" DestinationFolder="$(ProjectDir)/bin/publish/"/>
    <Copy SourceFiles="$(TargetDir)$(AssemblyName).dll" DestinationFolder="$(ProjectDir)/bin/publish/"/>

    <Message Text="Add to GAC" Importance="high"/>
    <Exec Command="gacutil /i $(TargetDir)$(AssemblyName).dll"/>
</Target>
```

### Com.Client

Client project.
Consists of one `.cpp` file, which describes the logic of working with the COM component

```c++
#import "../Com/bin/publish/Com.tlb" raw_interfaces_only

#include <iostream>

int main()
{
    HRESULT hr = CoInitialize(nullptr); // Initialize COM
    if (FAILED(hr))
    {
        std::cerr << "Failed to initialize COM." << std::endl;
        return -1;
    }

    std::cout << "Successfully initialized COM." << std::endl;

    try
    {
        // Create an instance of the COM class
        Com::IComInterfacePtr comObject;
        hr = comObject.CreateInstance(__uuidof(Com::ComInterface));
        if (FAILED(hr))
        {
            std::cerr << "Failed to create COM object. Status - " << std::hex << hr << std::endl;
            CoUninitialize();
            return -1;
        }

        std::cout << "Successfully created COM object." << std::endl;

        // Call the method
        long result = 0;
        comObject->Calculate(1, 1, &result);

        std::cout << "Calculation result is " << result << '\n';
    }
    catch (const _com_error& e)
    {
        std::cerr << "COM error: " << e.ErrorMessage() << std::endl;
    }

    CoUninitialize(); // Clean up COM
    return 0;
}
```

## Assembly

The project is built from any IDE or console.
As a result, 3 files `Com.dll`, `Com.tlb`, `Com.Client.exe` will be obtained

## Executing files

The COM component will be registered in the system, so information about it can be found using the command
`gci HKLM:\Software\Classes -ea 0| ? {$_.PSChildName -match '^\w+\.\w+$' -and (gp "$($_.PSPath)\CLSID" -ea 0)} | ft PSChildName | FindStr Com.ComInterface`.

You can also check using the command `gacutil.exe /lr | FindStr Com` that `.dll` was placed in the GAC

The following output will be obtained from the `.exe` run:

```console
Successfully initialized COM.
Successfully created COM object.
Calculation result is 2
```
