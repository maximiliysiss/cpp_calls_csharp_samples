namespace NewRuntime.Api;

public static class Api
{
    public delegate int CalculateDelegate(int a, int b);

    public static int Calculate(int a, int b) => a + b;
}
