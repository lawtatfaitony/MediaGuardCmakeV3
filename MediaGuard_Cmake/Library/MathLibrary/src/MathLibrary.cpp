#define MATH_LIBRARY_EXPORT
#include "MathLibrary.h"


int Add(int a, int b)
{
    return a + b;
}

int Div(int a, int b)
{
    if (0 == b) return -1;

    return a / b;
}