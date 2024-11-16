using System.Runtime.InteropServices;

namespace Com;

[ComVisible(true)]
[ClassInterface(ClassInterfaceType.None)]
public sealed class ComInterface : IComInterface
{
    public int Calculate(int a, int b) => a + b;
}
