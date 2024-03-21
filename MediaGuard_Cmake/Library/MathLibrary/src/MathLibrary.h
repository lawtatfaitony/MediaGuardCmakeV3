#pragma once


#ifdef WIN32
#ifdef MATH_LIBRARY_EXPORT
#define MATH_LIBRARY_API __declspec(dllexport)
#else
#define MATH_LIBRARY_API __declspec(dllimport)
#endif // HELLO_WORLD_EXPORT
#else
#define MATH_LIBRARY_API __attribute__((visibility("default")))
#endif // WIN32


MATH_LIBRARY_API int Add(int a, int b);

MATH_LIBRARY_API int Div(int a, int b);