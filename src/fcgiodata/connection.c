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
#include "common.h"

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#if defined(HAVE_POSIX)
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include <base/str.h>
#include <base/pam.h>
#include <base/chars.h>
#include <base/log.h>
#include <base/role.h>
#include <base/parse.h>
#include <base/http.h>
#include <base/dump.h>
#include <base/base64.h>
#include "phit.h"

#include "base/log.h"
#include "fcgiodata/connection.h"
#include "fcgiodata/context.h"

Connection* FCGI_ConnectionNew(zsock_t *socket, zmsg_t *msg)
{
    Connection* self;

    if (!(self = (Connection*)Calloc(1, sizeof(Connection))))
        return NULL;

    self->magic = CONNECTION_MAGIC;
    ContextInit(&self->context, self);

    self->socket = socket;
    self->msg = msg;
    self->return_identity = zmsg_first (self->msg); // client identity frame
    self->envp=NULL;
    self->content=NULL;

    AllocInit(&self->outAlloc, self->outBuffer, sizeof(self->outBuffer));
    AllocInit(&self->wbufAlloc, self->wbufBuffer, sizeof(self->wbufBuffer));

    PHIT_Context_DEBUG((PHIT_Context *)(&(self->context)), "ConnectionNew(%p)", self);

    BufInit(&self->out, &self->outAlloc);
    BufInit(&self->wbuf, &self->wbufAlloc);

    return self;
}

void FCGI_ConnectionDelete(
    Connection** self_)
{
    Connection *self = *self_;
    assert(self->magic == CONNECTION_MAGIC);
    PHIT_Context_DEBUG((PHIT_Context *)(&(self->context)), "ConnectionDelete(%p)", self);

    PHIT_Context_DEBUG((PHIT_Context *)(&(self->context)), "zmsg_destroy");
    zmsg_destroy (&self->msg);

    PHIT_Context_DEBUG((PHIT_Context *)(&(self->context)), "free content");
    if(self->content) {
        free(self->content);
        self->content = NULL;
    }

    PHIT_Context_DEBUG((PHIT_Context *)(&(self->context)), "free envp");
    if(self->envp) {
        for(int i=0; self->envp[i]; i++) {
            free(self->envp[i]);
            self->envp[i] = NULL;
        }
        free(self->envp);
        self->envp=NULL;
    }

    PHIT_Context_DEBUG((PHIT_Context *)(&(self->context)), "buf destroy out");
    BufDestroy(&self->out);
    PHIT_Context_DEBUG((PHIT_Context *)(&(self->context)), "buf destroy wbuf");
    BufDestroy(&self->wbuf);

    PHIT_Context_DEBUG((PHIT_Context *)(&(self->context)), "context destroy");
    ContextDestroy(&self->context);
    // can't call PHIT_Context_* functions after the destroy, above...

    self->magic = 0xDDDDDDDD;
    Free(self);
    self_ = NULL;
}
