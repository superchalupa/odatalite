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
#include <stdarg.h>
#include <base/buf.h>
#include <base/dump.h>
#include <base/log.h>
#include "context.h"
#include "connection.h"
#include "contextoptions.h"

// simple helper to reduce typing (cast looks somewhat ugly, no need to repeat it every time).
#define DEBUG_PRINTF(args...) PHIT_Context_DEBUG((PHIT_Context *)(&(c->context)), args)

static void _FCGI_Context_VLogMessage(
        PHIT_Context* context,
        int priority,
        const char *file,
        int line,
        const char *fn,
        const char *format,
        va_list args)
{
    Context* self = (Context*)context;
    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);
    (void)self;

    vsyslog(priority, format, args);
}

static void _FCGI_PostStatusLine(
    PHIT_Context* context,
    PHIT_StatusCode statusCode,
    const char* statusMsg)
{
    Context* self = (Context*)context;
    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);
    (void)self;

    char *name = "Status";
    char value[50] = "";

    snprintf(value, sizeof(value)-1, "%d %s", statusCode, statusMsg);
    context->PostHeader(context, name, value);
}



static void _PostHeaderAux(
    PHIT_Context* context,
    const char* name,
    size_t nameLen,
    const char* value,
    size_t valueLen)
{
    Context* self = (Context*)context;
    Buf* wbuf;

    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    wbuf = &self->connection->wbuf;
    nameLen = strlen(name);
    valueLen = strlen(value);

    BufCat(wbuf, name, nameLen);
    BufCat(wbuf, STRLIT(": "));
    BufCat(wbuf, value, valueLen);
    BufCat(wbuf, STRLIT("\r\n"));
}

static void _PostHeader(
    PHIT_Context* context,
    const char* name,
    const char* value)
{
    return _PostHeaderAux(
        context, name, strlen(name), value, strlen(value));
}

static void _PostHeaderUL(
    PHIT_Context* context,
    const char* name,
    unsigned long x)
{
    char buf[ULongToStrBufSize];
    const char* value;
    size_t valueLen;

    value = ULongToStr(buf, x, &valueLen);
    _PostHeaderAux(context, name, strlen(name), value, valueLen);
}

static void _PostTrailerField(
    PHIT_Context* context,
    const char* name,
    const char* value)
{
    Context* self = (Context*)context;
    Buf* out;
    size_t nameLen;
    size_t valueLen;

    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    out = &self->connection->out;
    nameLen = strlen(name);
    valueLen = strlen(value);

    BufCat(out, name, nameLen);
    BufCat(out, STRLIT(": "));
    BufCat(out, value, valueLen);
    BufCat(out, STRLIT("\r\n"));
}

static void _PostEOH(
    PHIT_Context* context)
{
    Context* self = (Context*)context;
    Buf* wbuf;

    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    wbuf = &self->connection->wbuf;

    /* If no "Content-Length" header, then use chunked encoding */
    if (self->acceptChunkedEncoding &&
        self->statusCode != PHIT_STATUSCODE_204_NO_CONTENT)
    {
        BufCat(wbuf, STRLIT("Transfer-Encoding: chunked\r\n"));
        self->chunkedEncoding = 1;
    }

    self->postedEOH = 1;

    if (self->chunkedEncoding)
        BufCat(wbuf, STRLIT("\r\n"));
}

static void _PostContent(
    PHIT_Context* context,
    const char* data,
    size_t size)
{
    Context* self = (Context*)context;
    Buf* out;

    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    out = &self->connection->out;

    if (data && size)
    {
        /* Write chunk prefix */
        if (self->chunkedEncoding)
            HTTPFormatChunk(out, data, size);
        else
            BufCat(out, data, size);
    }
}

static int send_response(zloop_t *loop, int timer_id, void *arg)
{
    Connection *c = (Connection *)arg;
    Context* ctx = (Context*)(&c->context);
    PHIT_Context_DEBUG((PHIT_Context *)(&(c->context)), "send_response\n");

    // return from timer: 0 == keep running timer, -1 == stop timer
    int ret = 0;

    int z_ret = 0;
    DEBUG_PRINTF("CHECK EOC: %d\n", ctx->postedEOC);

    if( ! ctx->postedEOC ) {
        DEBUG_PRINTF("Connection NOT complete, waiting: %d\n", timer_id);
        goto out_incomplete;
    }

    DEBUG_PRINTF("GOT EOC\n");

    // Create response message
    zmsg_t *response = zmsg_new();
    if(!response) {
        DEBUG_PRINTF("Error creating response message\n");
        goto error_sending;
    }

    // IDENTITY frame (dup existing because the original message still owns it)
    zframe_t *response_dest = zframe_dup(c->return_identity);
    if(!response_dest) {
        DEBUG_PRINTF("Error dup response identity\n");
        goto error_sending;
    }

    z_ret = zmsg_append( response, &response_dest );
    if(z_ret) {
        DEBUG_PRINTF("Error appending response identity\n");
        goto error_sending;
    }

    // NULL separator frame
    z_ret = zmsg_addstr( response, "" );
    if(z_ret) {
        DEBUG_PRINTF("Error appending null frame\n");
        goto error_sending;
    }

    // HEADER frame
    z_ret = zmsg_addmem( response, c->wbuf.data, c->wbuf.size );
    if(z_ret){
        DEBUG_PRINTF("Error appending http response headers\n");
        goto error_sending;
    }

    // CONTENT frame
    z_ret = zmsg_addmem( response, c->out.data, c->out.size );
    if(z_ret){
        DEBUG_PRINTF("Error appending http response content\n");
        goto error_sending;
    }

    // Actually send response message
    z_ret = zmsg_send(&response, c->socket);
    if(z_ret) {
        DEBUG_PRINTF("Error sending response message\n");
        goto error_sending;
    }

    ret = -1; // stop timer
    goto out_free;

error_sending:
    // normally zmq will take over message and destroy it, however if send
    // fails or we fail to add all our frames, we have to dispose of it
    // ourselves.
    DEBUG_PRINTF("ERROR sending message, destroying it.\n");
    zmsg_destroy(&response);
    // drop through below...

out_free:
    DEBUG_PRINTF("FCGI_ConnectionDelete\n");
    DEBUG_PRINTF("Processed message, returning. Ret=%d\n", ret);
    // no more DEBUG_PRINTF after connection delete!
    FCGI_ConnectionDelete(&c);
    // drop through below...

out_incomplete:
    if(ret == -1)
        zloop_timer_end (loop, timer_id);

    return 0;
}


static void _PostEOC(
    PHIT_Context* context)
{
    Context* self = (Context*)context;
    Buf* out;

    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    out = &self->connection->out;

    /* Write content length (in not chunked) */
    if (!self->chunkedEncoding)
    {
        Buf* wbuf = &self->connection->wbuf;
        _PostHeaderUL(context, "Content-Length", out->size);
        BufCat(wbuf, STRLIT("\r\n"));
    }

    /* Write the terminating null chunk */
    if (self->chunkedEncoding)
        BufCat(out, STRLIT("0\r\n"));

    self->plugin = NULL;
    self->postedEOC = 1;
    self->connection->chunkFinal = 1;

    // start up an immediate timer to send the results on the next async loop pass
    zloop_timer (self->connection->reactor, 1, 0, send_response, self->connection);
}

void _PostError(
    PHIT_Context* context,
    PHIT_StatusCode statusCode,
    const char* statusMsg,
    const char* detail)
{
    PHIT_Context_PostStatusLine(context, statusCode, statusMsg);
    PHIT_Context_PostHeader(context, "Content-Type", "text/html");
    PHIT_Context_PostEOH(context);

    PHIT_Context_PostContent(context, STRLIT(
        "<html><head></head>"
        "<body>"
        "<h2>"));

    PHIT_Context_PostContent(context, statusMsg, strlen(statusMsg));

    if (detail)
    {
        PHIT_Context_PostContent(context, STRLIT(": "));
        PHIT_Context_PostContent(context, detail, strlen(detail));
    }

    PHIT_Context_PostContent(context, STRLIT(
        "</h2>"
        "</body>"
        "</html>"));

    PHIT_Context_PostEOC(context);
}

static void* _SetPluginData(
    PHIT_Context* context,
    void* pluginData)
{
    Context* self = (Context*)context;
    void* oldPluginData;

    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    oldPluginData = self->pluginData;
    self->pluginData = pluginData;
    return oldPluginData;
}

static void* _GetPluginData(
    PHIT_Context* context)
{
    Context* self = (Context*)context;

    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    return self->pluginData;
}

static int _GetOption(
    const PHIT_Context* context,
    int option,
    void* value,
    size_t valueSize)
{
    Context* self = (Context*)context;

    if (option == CONTEXT_OPTION_BUF)
    {
        Buf* x;

        if (valueSize < sizeof(x) || !self->connection)
            return -1;

        x = &self->connection->out;
        memcpy(value, &x, sizeof(x));
        return 0;
    }

    if (option == CONTEXT_OPTION_CHUNKED_ENCODING)
    {
        int x = self->chunkedEncoding;

        if (valueSize < sizeof(x))
            return -1;

        memcpy(value, &x, sizeof(x));
        return 0;
    }

    if (option == PHIT_CONTEXT_OPTION_ROLE && self->connection)
    {
        PHIT_Role x = self->connection->role;

        if (valueSize < sizeof(x))
            return -1;

        memcpy(value, &x, sizeof(x));
        return 0;
    }

    /* Unknown option */
    return -1;
}

static int _SetLogPriority(
        PHIT_Context* context,
        int priority)
{
    Context* self = (Context*)context;
    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    int oldlevel = self->loglevel;
    self->loglevel = priority;
    return oldlevel;
}

static int _GetLogPriority(
        PHIT_Context* context)
{
    Context* self = (Context*)context;
    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    return self->loglevel;
}

struct _cb_args
{
    zmq_pollitem_t *p;
    int (*fn)(int, void *);
    void *arg;
};

#include <syslog.h>
static int _callback_shim (zloop_t *loop, zmq_pollitem_t *item, void *arg)
{
    struct _cb_args *a = (struct _cb_args *)arg;
    syslog(LOG_WARNING, "CALLBACK_SHIM!");
    /* callback should return -1 to end calls and free memory */
    int ret = a->fn(item->fd, a->arg);
    if (ret == -1)
    {
        syslog(LOG_WARNING, "CALLBACK_SHIM: remove fd %d!", item->fd);
        zloop_poller_end(loop, item);
        close(item->fd);
        free(a->p);
        free(a);
    }
    return 0;
}

static int _AddFDCallback(
    PHIT_Context* context,
    int fd,
    int (*fn)(int fd, void *arg),
    void *arg)
{
    Context* self = (Context*)context;

    PHIT_Context_DEBUG(context, "Hello from AddFDCallback: add fd %d\n", fd);
    struct _cb_args *a = calloc(1, sizeof(struct _cb_args));
    zmq_pollitem_t *poller = calloc(1, sizeof(zmq_pollitem_t));
    a->p = poller;
    a->fn = fn;
    a->arg = arg;

    // set up pollitem
    poller->fd = fd;
    poller->events = ZMQ_POLLIN;
    zloop_poller(self->connection->reactor, poller, _callback_shim, a);
    return 0;
}

static int _RemoveFDCallback(
    PHIT_Context* context, int fd)
{
    assert(1==0); /* this function is not yet safe to call as I need to figure out a way to free the memory held by the pollitem */

    Context* self = (Context*)context;
    zmq_pollitem_t poller = {NULL, fd, ZMQ_POLLIN };
    zloop_poller_end(self->connection->reactor, &poller);
    return 0;
}

static int _DeferResult(
    PHIT_Context* context)
{
    return 0;
}

static PHIT_Context _base =
{
    .magic = PHIT_CONTEXT_MAGIC,
    .PostStatusLine = _FCGI_PostStatusLine,
    .PostHeader = _PostHeader,
    .PostHeaderUL = _PostHeaderUL,
    .PostTrailerField = _PostTrailerField,
    .PostEOH = _PostEOH,
    .PostContent = _PostContent,
    .PostEOC = _PostEOC,
    .PostError = _PostError,
    .SetPluginData = _SetPluginData,
    .GetPluginData = _GetPluginData,
    .GetOption = _GetOption,
    .SetLogPriority = _SetLogPriority,
    .GetLogPriority = _GetLogPriority,
    .VLogMessage = _FCGI_Context_VLogMessage,
    .AddFDCallback = _AddFDCallback,
    .RemoveFDCallback = _RemoveFDCallback,
    .DeferResult = _DeferResult,
};

void ContextInit(
    Context* self,
    Connection* connection)
{
    memset(self, 0, sizeof(*self));
    self->base = _base;
    self->connection = connection;
    self->loglevel=9;
}

void ContextReset(
    Context* self)
{
    self->plugin = NULL;
    self->postedEOH = 0;
    self->postedEOC = 0;
    self->loglevel=9;
}

void ContextDestroy(
    Context* self)
{
    memset(self, 0xDD, sizeof(Context));
}
