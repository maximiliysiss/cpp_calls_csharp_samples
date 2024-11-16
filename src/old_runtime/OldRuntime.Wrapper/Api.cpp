#include "Api.h"
#include "Wrapper.h"

int calculate(int a, int b)
{
    return Wrapper::get_instance("OldRuntime.Api", "OldRuntime.Api.Api", "Calculate")->calculate(a, b);
}
