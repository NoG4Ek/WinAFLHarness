#ifndef HARNESS_C_
#define HARNESS_C_

#include <windows.h>
#include <winbase.h>
#include <windef.h>
#include <stdio.h>

typedef int(*TestFunc)(int, char**);

__declspec(dllexport) __declspec(noinline) int loadDLLFunc(int argc,
    char** argv,
    char const* library_name,
    char const* func_name,
    boolean should_redirect_output);

#endif
