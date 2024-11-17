# Framework .export instruction

## Description

This is probably one of the funniest ways to be able to call C# code from C++. To briefly describe the essence of the
solution - it is a modification of IL Code by adding `.export` instruction.

### Pros

1. Fun way :)
2. Supported out of the box and does not require additional compilers

### Cons

1. Difficult to implement - need to modify IL Code and call `ildasm`, `ilasm`
2. Only for Windows, as it only works on .NET Framework
3. Requires using GAC (when calling, it will use Runtime from it)
4. Requires .NET Runtime installed

### Useful links

1. [Ildasm.exe (IL Disassembler)](https://learn.microsoft.com/en-us/dotnet/framework/tools/ildasm-exe-il-disassembler)
2. [Ilasm.exe (IL Assembler)](https://learn.microsoft.com/en-us/dotnet/framework/tools/ilasm-exe-il-assembler)

## Projects

The solution consists of 2 projects:

1. `FrameworkExport` - a simple .NET library
2. `FrameworkExport.Client` - a client for the library

### FrameworkExport

A library with a simple API provided by `Api.cs`

```csharp
public static class Api
{
    public static int Calculate(int a, int b) => a + b;
}
```

All the magic is in the additional `Target`, which is run after the build.

```xml

<Target Name="ILRebuilding" AfterTargets="Build">
    <Message Text="Start rebuilding IL Code" Importance="high"/>

    <Message Text="Decompile IL Code" Importance="high"/>
    <Exec Command="ildasm /out:$(TargetDir)output.il $(TargetDir)$(AssemblyName).dll"/>

    <Message Text="Rewrite IL Code" Importance="high"/>
    <Exec
        Command="powershell -Command &quot;(gc $(TargetDir)output.il -Raw) -replace 'Calculate\(.*\n.*\n.*{', 'Calculate(int32 a,int32 b) cil managed { .export [1] as Calculate' | Out-File -FilePath $(TargetDir)output_il.il&quot;"/>

    <Message Text="Compile IL Code" Importance="high"/>
    <Exec Command="ilasm /out:$(TargetDir)$(AssemblyName)_il.dll /DLL /x64 /PE64 $(TargetDir)output_il.il"/>
</Target>
```

Here are the following steps:

1. Decompiling the assembled `.dll` using the command `ildasm /out:$(TargetDir)output.il $(TargetDir)$(AssemblyName).dll`
2. Modifying the IL Code. In fact, at the very beginning of the method (before `.maxstack`) an instruction of the format
   `.export[index] as name` is added (in the example it will be `.export[1] as Calculate`)
3. Assembling a new `.dll` using the command
   `ilasm /out:$(TargetDir)$(AssemblyName)_il.dll /DLL /x64 /PE64 $(TargetDir)output_il.il` for the x64 architecture

If we look at the code, here are the comparisons before and after:

```il
// BEFORE
.class public abstract auto ansi sealed beforefieldinit FrameworkExport.Api
       extends [mscorlib]System.Object
{
  .method public hidebysig static int32  Calculate(int32 a,
                                                   int32 b) cil managed
  {
    // Code size       4 (0x4)
    .maxstack  8
    IL_0000:  ldarg.0
    IL_0001:  ldarg.1
    IL_0002:  add
    IL_0003:  ret
  } // end of method Api::Calculate

} // end of class FrameworkExport.Api

// AFTER
.class public abstract auto ansi sealed beforefieldinit FrameworkExport.Api
       extends [mscorlib]System.Object
{
  .method public hidebysig static int32  Calculate(int32 a,
                                                   int32 b) cil managed
  {
    .export [1] as Calculate
    // Code size       4 (0x4)
    .maxstack  8
    IL_0000:  ldarg.0
    IL_0001:  ldarg.1
    IL_0002:  add
    IL_0003:  ret
  } // end of method Api::Calculate

} // end of class FrameworkExport.Api
```

### FrameworkExport.Client

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
As a result, several files will be obtained:

1. `FrameworkExport.dll` - original library
2. `FrameworkExport_il.dll` - rebuilt library with `.export`
3. `FrameworkExport.Client.dll` - client for the library

## Executing files

If you call `dumpbin /exports` on the wrapper library, you can get the following output, where you can see that the
`Calculate` method we defined is available for export.

```dumpbin
Dump of file FrameworkExport_il.dll

File Type: DLL

  Section contains the following exports for \FrameworkExport_il.dll

    00000000 characteristics
    673A0608 time date stamp Sun Nov 17 16:04:40 2024
        0.00 version
           1 ordinal base
           1 number of functions
           1 number of names

    ordinal hint RVA      name

          1    0 0000266A Calculate

  Summary

        2000 .reloc
        2000 .sdata
        2000 .text
```

If you execute the same command on the original file, you can get the following output:

```dumpbin
Dump of file FrameworkExport.dll

File Type: DLL

  Summary

        2000 .reloc
        2000 .rsrc
        2000 .text
```

When running `.exe` you may get the following output:

```console
Success LoadLibrary("FrameworkExport_il.dll")
Success GetProcAddress("Calculate")
Calculation result is 2
```
