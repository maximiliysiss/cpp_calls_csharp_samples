using System.Runtime.InteropServices;

namespace Com;

[ComVisible(true)]
[InterfaceType(ComInterfaceType.InterfaceIsDual)]
public interface IComInterface
{
    int Calculate(int a, int b);
}
