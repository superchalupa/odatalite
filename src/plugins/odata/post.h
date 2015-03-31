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
#ifndef _odata_plug_post_h
#define _odata_plug_post_h

#include <phit.h>
#include <odata/odata.h>
#include <base/str.h>
#include <stdarg.h>

void PostErrorV(
    PHIT_Context* context,
    OL_Result result,
    const char* format,
    va_list ap);

OL_PRINTF_FORMAT(3, 4)
void PostError(
    PHIT_Context* context,
    OL_Result result,
    const char* format,
    ...);

void PostStatusLineAndHeaders(
    PHIT_Context* context,
    PHIT_StatusCode statusCode,
    const char* statusMsg,
    const char* contentType);

void PostCountResponse(
    PHIT_Context* context,
    unsigned long count);

extern const StrLit __metadataV3[3];

extern const StrLit __metadataV4[3];

#endif /* _odata_plug_post_h */
