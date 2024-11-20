using System.Runtime.InteropServices;

namespace AheadOfTime;

public static class Api
{
    [UnmanagedCallersOnly(EntryPoint = "calculate")]
    public static int Calculate(int a, int b) => a + b;
}
