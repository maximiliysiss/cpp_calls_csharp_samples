#include "Api.h"
#include "Wrapper.h"

int calculate(int a, int b)
{
    return Wrapper::get_instance(L"NewRuntime.Api", L"NewRuntime.Api.Api, NewRuntime.Api", L"Calculate",
                                 L"NewRuntime.Api.Api+CalculateDelegate, NewRuntime.Api")->calculate(a, b);
}
