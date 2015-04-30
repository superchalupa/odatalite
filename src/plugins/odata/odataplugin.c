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
#include "post.h"
#include "odataplugin.h"
#include <phit.h>
#include <base/shlib.h>
#include <base/str.h>
#include <base/http.h>
#include <base/str.h>
#include <base/alloc.h>
#include <base/log.h>
#include <base/html.h>
#include <base/dump.h>
#include <base/path.h>
#include <base/role.h>
#include <odata/odata.h>
#include <odata/odatamacros.h>
#include <odata/scope.h>
#include <odata/outstream.h>
#include <odata/uri.h>
#include <odata/uriparser.h>
#include <odata/serialize.h>
#include <odata/result.h>
#include <server/contextoptions.h>
#include "providers.h"
#include "options.h"
#include "webpage.h"
#include "scope.h"

#ifdef ENABLE_DEBUG
#include <syslog.h>
#define DEBUG_OUT(fd, prio, args...) syslog(prio, args)
#define DEBUG_PRINTF(args...) DEBUG_OUT(stderr, LOG_INFO, args)
#else
#define DEBUG_OUT(fd, prio, args...)
#define DEBUG_PRINTF(args...)
#endif

/*
**=============================================================================
**
** Local definitions
**
**=============================================================================
*/

#if 0
static const StrLit _metadataV3[] =
{
    { STRLIT("nometadata") },
    { STRLIT("minimalmetadata") },
    { STRLIT("fullmetadata") }
};

static const StrLit _metadataV4[] =
{
    { STRLIT("none") },
    { STRLIT("minimal") },
    { STRLIT("full") }
};
#endif

static OL_Object* _Deserialize(
    OL_Scope* scope,
    char* data,
    size_t size)
{
    OL_Object* object = OL_Scope_NewObject(scope);

    if (object)
    {
        if (OL_Object_Deserialize(
            object,
            data,
            size,
            NULL) != OL_Result_Ok)
        {
            OL_Object_Release(object);
            return NULL;
        }
    }

    return object;
}

static int _ProcessAcceptHeader(
    const PHIT_Headers* headers,
    unsigned short odataVersion,
    OL_MetadataType* metadataType)
{
    /* Accept: application/json;odata.metadata=minimal */
    const char* accept;
    const char* p;
    const char* metadataVar = NULL;
    size_t metadataLen = 0;
    const StrLit* metadataArr = NULL;

    if (!(accept = PHIT_HeadersFind(headers, STRLIT("Accept"))))
        return -1;

    if ((p = Strnstr(accept, STRLIT("application/json"))))
    {
        p += STRN("application/json");
    }
    else if ((p = Strnstr(accept, STRLIT("*/*"))))
    {
        p += STRN("*/*");
    }
    else
    {
        /* Unsupported content type */
        return -1;
    }

    if (odataVersion == OL_VERSION_4_0)
    {
        metadataArr = __metadataV4;
        metadataVar = "odata.metadata=";
        metadataLen = STRN("odata.metadata=");
    }
    else if (odataVersion == OL_VERSION_3_0)
    {
        metadataArr = __metadataV3;
        metadataVar = "odata=";
        metadataLen = STRN("odata=");
    }

    if (metadataVar && (p = Strnstr(p, metadataVar, metadataLen)))
    {
        size_t i;
        p += metadataLen;

        for (i = 0; i < 3; i++)
        {
            if (strncmp(p, metadataArr[i].str,  metadataArr[i].len) == 0)
            {
                *metadataType = (OL_MetadataType)i;
                return 0;
            }
        }

        /* Not found */
        return -1;
    }

    return 0;
}

static int _ProcessVersionHeader(
    const PHIT_Headers* headers,
    unsigned short* odataVersion)
{
    const char* value;

    /* Version 4.0 */
    if ((value = PHIT_HeadersFind(headers, STRLIT("OData-Version"))) &&
        strcmp(value, "4.0") == 0)
    {
        *odataVersion = OL_VERSION_4_0;
        return 0;
    }

    /* Version 3.0 */
    if ((value = PHIT_HeadersFind(headers, STRLIT("DataServiceVersion"))) &&
        strcmp(value, "3.0") == 0)
    {
        *odataVersion = OL_VERSION_3_0;
        return 0;
    }

    /* If OData version headers missing, deduce version from metadata level */
    if ((value = PHIT_HeadersFind(headers, STRLIT("Accept"))))
    {
        if (Strnstr(value, STRLIT("nometadata")) ||
            Strnstr(value, STRLIT("minimalmetadata")) ||
            Strnstr(value, STRLIT("fullmetadata")))
        {
            *odataVersion = OL_VERSION_3_0;
        }
        else
        {
            *odataVersion = OL_VERSION_4_0;
        }
        return 0;
    }

    return -1;
}

/*
**=============================================================================
**
** Plugin
**
**=============================================================================
*/

static void _ODATAPlugin_Load(
    PHIT_Plugin* plugin)
{
    DEBUG_PRINTF("=== OL_Load()\n");

    /* Load all providers */
    ProvidersLoad();
}

static void _ODATAPlugin_Unload(
    PHIT_Plugin* plugin)
{
    DEBUG_PRINTF("=== OL_Unload()\n");

    /* Unload all providers */
    ProvidersUnload();

    ScopeCache_Release();
}

static void _ODATAPlugin_HandlePull(
    PHIT_Plugin* plugin,
    PHIT_Context* context);

static void _ODATAPlugin_HandleRequest(
    PHIT_Plugin* plugin,
    PHIT_Context* context,
    PHIT_Method method,
    const char* requestURI,
    const PHIT_Headers* headers,
    const char* content,
    size_t contentLength)
{
    ODATAPlugin* self = (ODATAPlugin*)plugin;
    OL_Scope* scope = NULL;
    URI* uri = NULL;
    char err[64];
    OL_MetadataType metadataType = OL_MetadataType_Minimal;
    unsigned short version;

    PHIT_Context_DEBUG(context, "=== OL_HandleRequest(%s)\n", requestURI);

    /* Check whether requestor wants a web page */
    if (strcmp(requestURI, "/odata/webpage") == 0)
    {
        PostWebPage(plugin, context);
        return;
    }
    else if (strcmp(requestURI, "/odata/providers.conf") == 0)
    {
        char buf[PATH_MAX];
        MakePath(ID_PROVIDERS_CONF, buf);
        HTMLPostFile(context, buf, "text/plain");
        return;
    }

    /* Process the OData version header */
    if (_ProcessVersionHeader(headers, &version) != 0)
    {
        PostError(context, OL_Result_InternalError,
            "unsupported OData version");
        goto failed;
    }

    /* Process 'Accept' */
    if (_ProcessAcceptHeader(headers, version, &metadataType) != 0)
    {
        PostError(context, OL_Result_InternalError, "bad 'Accept' header");
        goto failed;
    }

    PHIT_Context_DEBUG(context, "metadataType{%u}\n", metadataType);

    /* Create context for this request */
    if (!(scope = ScopeCache_Get()))
    {
        PostError(context, OL_Result_InternalError, "_ScopeCache_Get");
        goto failed;
    }

    /* Set Scope.metadataType */
    ((Scope*)scope)->metadataType = metadataType;

    /* Set Scope.phitContext */
    ((Scope*)scope)->phitContext = context;

    /* Set Scope.version and Scope.foundVersion */
    ((Scope*)scope)->version = version;

    /* Make a copy of the request URI */
    if (!(((Scope*)scope)->requestURI = Strdup(requestURI)))
    {
        PostError(context, OL_Result_InternalError, "out of memory");
        goto failed;
    }

    if (!(uri = (URI*)OL_Scope_NewURI(scope)))
    {
        PHIT_Context_DEBUG(context, "failed to create URI");
        PostError(context, OL_Result_InternalError, "failed to create URI");
        goto failed;
    }

    /* The two contexts will refer to each other */
    PHIT_Context_SetPluginData(context, scope);

    // Pass in the ServiceRoot from the headers, if found.
    const char *serviceRoot = headers->ServiceRoot.found ? 
                            headers->ServiceRoot.value : DEFAULT_SERVICE_ROOT;
    strncpy(uri->service, serviceRoot, sizeof(uri->service));
    uri->serviceLen = strlen(uri->service);
    
    /* Parse the URI (destroy requestURI in place) */
    if (URIParse(uri, (char*)requestURI, err, sizeof(err)) != OL_Result_Ok)
    {
        PHIT_Context_DEBUG(context, "failed to parse URI: %s", requestURI);
        PostError(context, OL_Result_Failed, "failed to parse URI");
        goto failed;
    }

    /* Get the entitySetName */
    if (uri->segments.size)
    {
        const char* name = OL_URI_GetName(&uri->base, 0);

        if (!name)
        {
            PostError(context, OL_Result_Failed, "invalid URI");
            PHIT_Context_ERR(context, "URI has no $0: '%s'", requestURI);
            goto failed;
        }

        /* We know that this is preceded with a '/' character */
        DEBUG_ASSERT(name[-1] == '/');
    }

    /* Find provider entry and set up paging */
    {
        ProviderEntry* entry;
        size_t numMatchingSegments;

        /* Find provider for this request */
        if (!(entry = FindProviderEntry(uri, &numMatchingSegments)))
        {
            char errstr[1024] = {0};
            snprintf(errstr, 1023, "no provider for URI: '%s'", requestURI);
            PostError(context, OL_Result_NotFound, "%s", errstr);
            goto failed;
        }

        /* ATTN: are static properties supported? e.g., /service/Entity/Prop */

        /* ATTN: allow provider to handle property requests */

        /* Remove unused final segment from the URI since it is not handled
         * by the provider and probably indicates a property request.
         */
        if (numMatchingSegments < uri->segments.size)
        {
            if (uri->segments.size - numMatchingSegments > 1)
            {
                char errstr[1024] = {0};
                snprintf(errstr, 1023, "no provider for URI: '%s'", requestURI);
                PostError(context, OL_Result_NotFound, "%s", errstr);
                goto failed;
            }

            /* Save the segment name to process property requests later */
            ((Scope*)scope)->unmatchedSegmentName =
                uri->segments.data[uri->segments.size-1].name;
        }

        PHIT_Context_INFO(context, "selected provider %s", entry->libname);

        self->provider = entry->provider;

        /* Save the URI question options in the scope */
        ((Scope*)scope)->uriQueryOptions = uri->queryOptions;

        /* Set up count query option */
        if (uri->queryOptions & OL_QUERY_OPTION_COUNT &&
           !(entry->queryOptions & OL_QUERY_OPTION_COUNT))
        {
            ((Scope*)scope)->isCountRequest = OL_TRUE;
        }

        /* Set up paging options ($top and $skip) */
        {
            if (uri->queryOptions & OL_QUERY_OPTION_TOP &&
               !(entry->queryOptions & OL_QUERY_OPTION_TOP))
            {
                /* Provider will not handle top */
                ((Scope*)scope)->top = uri->queryOptionTop;
            }

            if (uri->queryOptions & OL_QUERY_OPTION_SKIP &&
               !(entry->queryOptions & OL_QUERY_OPTION_SKIP))
            {
                /* Provider will not handle skip */
                ((Scope*)scope)->skip = uri->queryOptionSkip;
            }

            ((Scope*)scope)->count = 0;
        }
    }

    /* Format the context URL; use the ServiceRoot from the phit headers */
    if (URIFormatContextURL(
        uri,
        serviceRoot,
        ((Scope*)scope)->contextURI,
        sizeof(((Scope*)scope)->contextURI)) != OL_Result_Ok)
    {
        PostError(context, OL_Result_NotSupported, "Unknown URI format");
        goto failed;
    }

    /* ODATA scope will need access to PHIT context later */
    ((Scope*)scope)->privateData = context;

    if (method == PHIT_METHOD_GET)
    {
        ((Scope*)scope)->httpStatusCode = PHIT_STATUSCODE_200_OK;
        ((Scope*)scope)->httpStatusMsg = PHIT_STATUSMSG_200_OK;

        PHIT_Context_DEBUG(context, "call provider GET");
        (*self->provider->ft->Get)(
            self->provider,
            scope,
            &uri->base);
        PHIT_Context_DEBUG(context, "done with provider GET");
    }
    else if (method == PHIT_METHOD_POST)
    {
        OL_Object* object =
            _Deserialize(scope, (char*)content, contentLength);

        ((Scope*)scope)->httpStatusCode = PHIT_STATUSCODE_201_CREATED;
        ((Scope*)scope)->httpStatusMsg = PHIT_STATUSMSG_201_CREATED;

        if (!object)
        {
            PostError(context, OL_Result_Failed, "invalid request body");
            goto failed;
        }

        (*self->provider->ft->Post)(
            self->provider,
            scope,
            &uri->base,
            object);

        OL_Object_Release(object);
    }
    else if (method == PHIT_METHOD_PUT)
    {
        OL_Object* object =
            _Deserialize(scope, (char*)content, contentLength);

        ((Scope*)scope)->httpStatusCode = PHIT_STATUSCODE_201_CREATED;
        ((Scope*)scope)->httpStatusMsg = PHIT_STATUSMSG_201_CREATED;

        if (!object)
        {
            PostError(context, OL_Result_Failed, "invalid request body");
            goto failed;
        }

        (*self->provider->ft->Put)(
            self->provider,
            scope,
            &uri->base,
            object);

        OL_Object_Release(object);
    }
    else if (method == PHIT_METHOD_PATCH)
    {
        OL_Object* object =
            _Deserialize(scope, (char*)content, contentLength);

        ((Scope*)scope)->httpStatusCode = PHIT_STATUSCODE_204_NO_CONTENT;
        ((Scope*)scope)->httpStatusMsg = PHIT_STATUSMSG_204_NO_CONTENT;

        if (!object)
        {
            PostError(context, OL_Result_Failed, "invalid request body");
            goto failed;
        }

        (*self->provider->ft->Patch)(
            self->provider,
            scope,
            &uri->base,
            object);

        OL_Object_Release(object);
    }
    else if (method == PHIT_METHOD_DELETE)
    {
        ((Scope*)scope)->httpStatusCode = PHIT_STATUSCODE_204_NO_CONTENT;
        ((Scope*)scope)->httpStatusMsg = PHIT_STATUSMSG_204_NO_CONTENT;

        (*self->provider->ft->Delete)(
            self->provider,
            scope,
            &uri->base);
    }
    else if (method == PHIT_METHOD_OPTIONS)
    {
        PHIT_Context_PostStatusLine(
            context, PHIT_STATUSCODE_200_OK, PHIT_STATUSMSG_200_OK);

        PHIT_Context_PostHeader(context, "Allow",
            "GET,POST,PUT,PATCH,DELETE,OPTIONS");

        PHIT_Context_PostHeader(context, "Public",
            "GET,POST,PUT,PATCH,DELETE,OPTIONS");

        PHIT_Context_PostHeader(context, "Access-Control-Allow-Origin", "*");

        PHIT_Context_PostHeader(context, "Access-Control-Allow-Methods",
            "GET,POST,PUT,PATCH,DELETE,OPTIONS");

        PHIT_Context_PostHeader(context, "Access-Control-Allow-Headers",
            "DataServiceVersion,OData-Version,Accept,MaxDataServiceVersion");

        PHIT_Context_PostHeader(context, "Access-Control-Expose-Headers", "*");

        PHIT_Context_PostEOH(context);
        PHIT_Context_PostEOC(context);
    }
    else
    {
        PostError(context, OL_Result_NotSupported,
            "HTTP method: %s", HTTPMethodNameOf(method));
    }

    PHIT_Context_DEBUG(context, "Release URI in handlerequest");
    if (uri)
    {
        PHIT_Context_DEBUG(context, "\tdoit");
        OL_URI_Release(&uri->base);
        PHIT_Context_DEBUG(context, "\tdone");
    }

#if 0
    /* ATTN.A: hack to handle pulls when client does not accept chunking */
    if (method == PHIT_METHOD_GET &&
        !((Scope*)scope)->postResult &&
        !(headers->te.found && headers->te.chunked))
    {
        while (!((Scope*)scope)->postResult)
        {
            _ODATAPlugin_HandlePull(plugin, context);
        }
    }
#endif

    PHIT_Context_DEBUG(context, "handlerequest RETURN");
    return;

failed:

    if (uri)
        OL_URI_Release(&uri->base);

    if (scope)
        ScopeCache_Put(scope);
}

static void _ODATAPlugin_HandlePull(
    PHIT_Plugin* plugin,
    PHIT_Context* context)
{
}

EXPORT ODATAPlugin __odataPlugin =
{
    {
        _ODATAPlugin_Load,
        _ODATAPlugin_Unload,
        _ODATAPlugin_HandleRequest,
        _ODATAPlugin_HandlePull
    },
    NULL, /* providerShlib */
    NULL, /* provider */
    OL_FALSE /* calledProviderLoad */
};
