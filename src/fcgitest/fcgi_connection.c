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
** Permission is hereby granted, free of charge, to any person obtaining a copy ** of this software and associated documentation files (the ""Software""), to
** deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
** sell copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions: The above copyright ** notice and this permission notice shall be included in all copies or
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
#include "common.h"

#include "base/log.h"
#include "fcgi_connection.h"

Connection* FCGI_ConnectionNew()
{
    Connection* self;

    if (!(self = (Connection*)Calloc(1, sizeof(Connection))))
        return NULL;

    self->magic = CONNECTION_MAGIC;

    AllocInit(&self->inAlloc, self->inBuffer, sizeof(self->inBuffer));
    AllocInit(&self->outAlloc, self->outBuffer, sizeof(self->outBuffer));

    LOGD(("ConnectionNew(%p)", self));

    BufInit(&self->in, &self->inAlloc);
    BufInit(&self->out, &self->outAlloc);
    BufInit(&self->wbuf, &self->wbufAlloc);

    ContextInit(&self->context, self);

    return self;
}

void FCGI_ConnectionDelete(
    Connection* self)
{
    LOGD(("ConnectionDelete(%p)", self));

    BufDestroy(&self->in);
    BufDestroy(&self->out);
    BufDestroy(&self->wbuf);
    ContextDestroy(&self->context);

    self->magic = 0xDDDDDDDD;
    Free(self);
}