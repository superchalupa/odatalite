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
#define _GNU_SOURCE

#include <syslog.h>

#include "log.h"
#include "datetime.h"
#include "path.h"
#include "cleanup.h"
#include "str.h"
#include "file.h"

#if defined(ENABLE_LOGGING)

static const char *lognames[] = {
    "EMERG", // = 0
    "ALERT", // = 1
    "CRIT", // = 2
    "ERR", // = 3
    "WARNING", // = 4
    "NOTICE", // = 5
    "INFO", // = 6
    "DEBUG", // = 7
    NULL
};

LogLevel __logLevel = LOG_WARNING;

FILE* __log;

static void _CleanupCallback(
    CleanupHandler* handler)
{
    LogClose();
}

static CleanupHandler _cleanupHandler;

int LogLevelFromString(
    const char* str,
    LogLevel* level)
{
    size_t i;

    for (i = 0; lognames[i];  i++)
    {
        if (Strcasecmp(lognames[i] , str) == 0)
        {
            *level = (LogLevel)i;
            return 0;
        }
    }

    /* Not found */
    return -1;
}

FILE* LogGetStream()
{
    char buf[MAX_PATH_SIZE];

    if (!__log)
    {
        if (!(__log = Fopen(MakePath(ID_LOGFILE, buf), "a")))
        {
            __log = stderr;
            return stderr;
        }
    }

    if (!_cleanupHandler.callback)
    {
        _cleanupHandler.callback = _CleanupCallback;
        AddCleanupHandler(&_cleanupHandler);
    }

    return __log;
}

void LogSetStream(FILE* os)
{
    if (__log && __log != os)
        fclose(__log);

    __log = os;
}

void VLog(
    LogLevel level,
    const char* format,
    va_list ap)
{
    if (level >= __logLevel)
        return;

    if (!__log)
    {
        LogGetStream();

        if (!__log)
        {
            /* Fall back on standard error */
            __log = stdout;
        }
    }

#if defined(ENABLE_DATETIME_LOGGING)
    {
        char timestamp[TIMESTAMP_SIZE];

        *timestamp = '\0';

        if (GetTimeStamp(timestamp) == 0)
        {
            fprintf(__log, "%s: ", timestamp);
        }
    }
#endif /* defined(ENABLE_DATETIME_LOGGING) */

#if defined(ENABLE_LOGLEVEL_LOGGING)
    fprintf(__log, "%s: ", lognames[level]);
#else
    fprintf(__log, "%c: ", lognames[level][0]);
#endif
    vfprintf(__log, format, ap);
    fputc('\n', __log);
    fflush(__log);
}

#if defined(INCLUDE_UNUSED)
PRINTF_FORMAT(2, 3)
void Log(
    LogLevel level,
    const char* format,
    ...)
{
    va_list ap;
    va_start(ap, format);
    VLog(level, format, ap);
    va_end(ap);
}
#endif /* defined(INCLUDE_UNUSED) */

void LogClose()
{
    if (__log && __log != stdout && __log != stderr)
    {
        fclose(__log);
        __log = NULL;
    }
}

#if defined(INCLUDE_UNUSED)
void __LogF(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    VLog(LOG_EMERG, format, ap);
    va_end(ap);
}
#endif /* defined(INCLUDE_UNUSED) */

#if 1 /* defined(INCLUDE_UNUSED) */
void __LogE(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    VLog(LOG_ERR, format, ap);
    va_end(ap);
}
#endif /* defined(INCLUDE_UNUSED) */

void __LogW(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    VLog(LOG_WARNING, format, ap);
    va_end(ap);
}

void __LogI(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    VLog(LOG_INFO, format, ap);
    va_end(ap);
}

#if defined(ENABLE_DEBUG)

void __LogD(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    VLog(LOG_DEBUG, format, ap);
    va_end(ap);
}

#endif /* defined(ENABLE_DEBUG) */

#endif /* defined(ENABLE_LOGGING) */
