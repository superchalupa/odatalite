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
#ifndef _connection_h
#define _connection_h

#include <stdlib.h>
#include <czmq.h>

#include <base/alloc.h>
#include <base/buf.h>
#include <base/sock.h>
#include <base/selector.h>
#include <base/auth.h>
#include <base/addr.h>
#include <base/http.h>
#include "context.h"

#if defined(ENABLE_STATIC_BUFFERS)
# define INBUF_CAPACITY 1024
# define OUTBUF_CAPACITY 1024
#else
# define INBUF_CAPACITY 1
# define OUTBUF_CAPACITY 1
#endif

#define USE_CMD

#define IN_BUF_SIZE 4096

#define CONNECTION_MAGIC 0x69027113

struct _Connection
{
    Handler base;

    unsigned int magic;

    /* Interim buffer (eventually copied to wbuf) */
    Buf out;
    Alloc outAlloc;
    char outBuffer[1024];

    /* Write buffer (all writes done directly from this buffer) */
    Buf wbuf;
    Alloc wbufAlloc;
    char wbufBuffer[1024];

    /* Provider invocation context */
    Context context;

    zloop_t *reactor;
    zsock_t *socket;
    zmsg_t *msg;
    zframe_t *return_identity;
    char **envp;
    char *content;
    PHIT_Headers headers;

    /* Authenticated user */
    char user[USERNAME_SIZE];

    /* Authenticated role */
    PHIT_Role role;

    /* Whether this is the final chunk */
    int chunkFinal;
};

Connection* FCGI_ConnectionNew(zloop_t *reactor, zsock_t *socket, zmsg_t *msg);
void FCGI_ConnectionDelete( Connection** self);

#endif /* _connection_h */
