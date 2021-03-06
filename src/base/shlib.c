/*
**==============================================================================
**
** ODatatLite ver. 0.0.3
**
** Copyright (c) Microsoft Corporation
**
** All rights reserved.
**
** MIT License
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the ""Software""), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions: The above copyright
** notice and this permission notice shall be included in all copies or
** substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
**==============================================================================
*/
#include "shlib.h"
#include <dlfcn.h>

// config.h should be included last, followed by anything that relies on #defines in config.h
#include "config.h"

Shlib* ShlibOpen(const char* path)
{
#if defined(HAVE_POSIX)
    int flags = RTLD_LAZY | RTLD_LOCAL;

# if defined(__linux)
    flags |= RTLD_DEEPBIND;
# endif
    return dlopen(path, flags);
#else
    return (Shlib*)LoadLibraryExA(path, NULL, 0);
#endif
}

int ShlibClose(Shlib* self)
{
#if defined(HAVE_POSIX)
    return dlclose(self);
#else
    return FreeLibrary((HMODULE)self) ? 0 : -1;
#endif
}


void* ShlibSym(
    Shlib* self,
    const char* symbol)
{
#if defined(_MSC_VER)
# pragma warning(disable:4054)
# pragma warning(disable:4055)

    FARPROC result;
    result = GetProcAddress((HMODULE)self, symbol);
    return (void*)result;
#else
    return dlsym(self, symbol);
#endif
}

size_t ShlibErr(char* buf, size_t bufSize)
{
#if defined(_MSC_VER)
    char* err = NULL;
    size_t r;

    if (!FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER|
        FORMAT_MESSAGE_FROM_SYSTEM|
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&err,
        0,
        NULL))
    {
        return 0;
    }

    r = Strlcpy(buf, err, bufSize);
    LocalFree(err);
    return r;
#else

    char* err;

    if (!(err = dlerror()))
        return 0;

    return Strlcpy(buf, err, bufSize);
#endif
}

int MakeShlibName(
    char buf[PATH_MAX],
    const char* libname)
{
    if (Strlcpy3(
        buf,
        SHLIB_PREFIX,
        libname,
        SHLIB_SUFFIX,
        PATH_MAX) >= PATH_MAX)
    {
	return -1;
    }

    return 0;
}
