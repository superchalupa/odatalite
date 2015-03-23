/*
 * echo.c --
 *
 *	Produce a page containing all FastCGI inputs
 *
 *
 * Copyright (c) 1996 Open Market, Inc.
 *
 * See the file "LICENSE.TERMS" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

extern char **environ;

#define NO_FCGI_DEFINES
#include "fcgi_stdio.h"
#include "src/plugins/odata/odataplugin.h"
#include "fcgitest/context.h"
#include "fcgitest/connection.h"
#include "server/contextoptions.h"


/* Post HTTP status line */
static void _PostStatusLine(
        PHIT_Context* context,
        PHIT_StatusCode statusCode,
        const char* statusMsg)
{
    /* TODO */
    printf("STATUSMSG: %s\n", statusMsg);
}

/* Post HTTP header field */
static void _PostHeader(
        PHIT_Context* context,
        const char* name,
        const char* value)
{
    /* TODO */
   printf("%s: %s\n", name, value);
}

/* Post HTTP header field */
static void _PostHeaderUL(
        PHIT_Context* context,
        const char* name,
        unsigned long value)
{
    /* TODO */
   printf("%s: %lu\n", name, value);
}

/* Post HTTP header field */
static void _PostTrailerField(
        PHIT_Context* context,
        const char* name,
        const char* value)
{
    /* TODO */
   printf("%s: %s\n", name, value);
}

/* Post end of headers */
static void _PostEOH(
        PHIT_Context* context)
{
   printf("END OF HEADERS\n");
}

/* Post HTTP content */
static void _PostContent(
        PHIT_Context* context,
        const char* data,
        size_t size)
{
    printf("%s\n", data);
}

/* Post end of content */
static void _PostEOC(
        PHIT_Context* context)
{
   printf("END OF CONTENT\n");
}

/* Post an error message */
static void _PostError(
        PHIT_Context* context,
        PHIT_StatusCode statusCode,
        const char* statusMsg,
        const char* detail)
{
    /* TODO */
    printf("ERROR: %s: %s\n", statusMsg, detail);
}

/* Set client data */
static void* _SetPluginData(
        PHIT_Context* context,
        void* pluginData)
{
    Context* self = (Context*)context;
    void* oldPluginData;

    printf("_SetPluginData\n");

    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);

    oldPluginData = self->pluginData;
    self->pluginData = pluginData;
    return oldPluginData;
}


/* Get client data */
static void* _GetPluginData(
        PHIT_Context* context)
{
    Context* self = (Context*)context;
    printf("_GetPluginData\n");
    DEBUG_ASSERT(context->magic == PHIT_CONTEXT_MAGIC);
    return self->pluginData;
}


/* Get option */
static int _GetOption(
        const PHIT_Context* context,
        int option,
        void* value,
        size_t valueSize)
{
    Context* self = (Context*)context;
    int ret = -1;

    if (option == CONTEXT_OPTION_BUF)
    {
        printf("_GetOption: CONTEXT_OPTION_BUF\n");
        Buf* x;


        if (valueSize < sizeof(x) || !self->connection)
            goto error_out;

        x = &self->out;
        memcpy(value, &x, sizeof(x));

        printf("DEBUG CURRENT BUF: %s\n", x->data);
        ret = 0;
        goto out;
    }

    else if (option == CONTEXT_OPTION_CHUNKED_ENCODING)
    {
        printf("_GetOption: CONTEXT_OPTION_CHUNKED_ENCODING\n");
        int x = self->chunkedEncoding;

        if (valueSize < sizeof(x))
            goto error_out;

        memcpy(value, &x, sizeof(x));
        ret = 0;
        goto out;
    }

    else if (option == PHIT_CONTEXT_OPTION_ROLE && self->connection)
    {
        PHIT_Role x = self->connection->role;
        printf("_GetOption: PHIT_CONTEXT_OPTION_ROLE\n");

        if (valueSize < sizeof(x))
            goto error_out;

        memcpy(value, &x, sizeof(x));
        ret = 0;
        goto out;
    }

    printf("_GetOption: unknown option\n");

error_out:
    ret = -1;
    printf("_GetOption error\n");

out:
    /* Unknown option */
    return ret;
}

static PHIT_Context _fastcgi_context = {
    .magic = PHIT_CONTEXT_MAGIC,
    .PostStatusLine = &_PostStatusLine,
    .PostHeader = &_PostHeader,
    .PostHeaderUL = &_PostHeaderUL,
    .PostTrailerField = &_PostTrailerField,
    .PostEOH = &_PostEOH,
    .PostContent = &_PostContent,
    .PostEOC = &_PostEOC,
    .PostError = &_PostError,
    .SetPluginData = &_SetPluginData,
    .GetPluginData = &_GetPluginData,
    .GetOption = &_GetOption,
};

void ContextInit(
    Context* self,
    Connection* connection)
{
    memset(self, 0, sizeof(*self));
    self->base = _fastcgi_context;
    self->connection = connection;
    AllocInit(&self->outAlloc, self->outBuffer, sizeof(self->outBuffer));
    BufInit(&self->out, &self->outAlloc);
}

void ContextReset(
    Context* self)
{
    self->plugin = NULL;
    self->postedEOH = 0;
    self->postedEOC = 0;
    BufDestroy(&self->out);
    BufInit(&self->out, &self->outAlloc);
}

void ContextDestroy(
    Context* self)
{
    BufDestroy(&self->out);
    memset(self, 0xDD, sizeof(Context));
}
