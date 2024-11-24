#include "Api.h"
#include "Wrapper.h"

int calculate(int a, int b)
{
    return Wrapper::get_instance(L"NewRuntimeDelegateFree.Api", L"NewRuntimeDelegateFree.Api.Api, NewRuntimeDelegateFree.Api", L"Calculate")
        ->calculate(a, b);
}
