using System.Runtime.InteropServices;

namespace NewRuntimeDelegateFree.Api;

public static class Api
{
    [UnmanagedCallersOnly]
    public static int Calculate(int a, int b) => a + b;
}
