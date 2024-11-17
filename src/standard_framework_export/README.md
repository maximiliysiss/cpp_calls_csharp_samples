# Using Framework export like wrapper for .NET Standard library

## Description

Similar solution to the `framework_export` project. For implementation details, see
[README](../framework_export/README.md).

This solution is an example of using a .NET Framework wrapper for a .NET Standard library. Wrapper on .NET
Framework uses IL Code rewrites to provide native exports.

## Projects

The solution consists of 3 projects:

1. `StandardFrameworkExport.Api` - API on .NET Standard
2. `StandardFrameworkExport.Client` - client
3. `StandardFrameworkExport.Wrapper` - wrapper for .NET Standard

## Executing files

When calling `dumpbin /exports` on `StandardFrameworkExport.Api.dll` you may get output that does not contain any
information about the exports:

```dumpbin
Dump of file StandardFrameworkExport.Api.dll

File Type: DLL

  Summary

        2000 .reloc
        2000 .rsrc
        2000 .text
```

If you call a similar command for `StandardFrameworkExport.Wrapper.dll`, you can get the output with the required
export:

```dumpbin
Dump of file StandardFrameworkExport.Wrapper_il.dll

File Type: DLL

  Section contains the following exports for \StandardFrameworkExport.Wrapper_il.dll

    00000000 characteristics
    673A0C5A time date stamp Sun Nov 17 16:31:38 2024
        0.00 version
           1 ordinal base
           1 number of functions
           1 number of names

    ordinal hint RVA      name

          1    0 000026DA Calculate

  Summary

        2000 .reloc
        2000 .sdata
        2000 .text
```

When running `.exe` you may get the following output:

```console
Success LoadLibrary("StandardFrameworkExport.Wrapper_il.dll")
Success GetProcAddress("Calculate")
Calculation result is 2
```
