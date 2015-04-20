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
#include <string.h>

#include <odata/odata.h>
#include "base/http.h"
#include "base/parse.h"
#include "base/str.h"
#include "base/chars.h"
#include "base/dump.h"

# define T(X)
# define D(X)
#define DEBUG_PRINTF(args...) DEBUG_OUT(stderr, LOG_INFO, args)
#if 0
#include <syslog.h>
#define DEBUG_OUT(fd, prio, args...) syslog(prio, args)
#else
#define DEBUG_OUT(fd, prio, args...)
#endif


static int _AppendHeader(
    PHIT_Header* headers,
    size_t* nheaders,
    const char* name,
    const char* value)
{
    if (*nheaders == HEADERS_BUFSIZE)
        return -1;

    headers[*nheaders].name = name;
    headers[*nheaders].value = value;
    (*nheaders)++;

    return 0;
}

static int _HTTPParseContentLength(
    PHIT_Headers* headers,
    char* data,
    size_t size)
{
    char* end;
    headers->contentLength.found = 1;
    headers->contentLength.value = strtoul(data, &end, 10);
    return data == end ? -1 : 0;
}

/*
    parameter = attribute "=" value
    attribute = token
    value = token | quoted-string
    token = 1*<any CHAR except CTLs or separators>
*/
static char* _ParseHTTPParameter(
    char* p,
    PHIT_HTTPParameter* param)
{
    /* Get the name */
    {
        char* start;
        char* end;

        while (IsLWS(*p))
            p++;

        if (!*p)
            return p;

        start = p;

        while (IsHTTPToken(*p))
            p++;

        if (p == start)
            return NULL;

        end = p;

        while (IsLWS(*p))
            p++;

        if (*p++ != '=')
            return NULL;

        *end = '\0';
        param->name = start;
    }

    /* Get the value */
    {
        char* start;
        char* end;

        while (IsLWS(*p))
            p++;

        if (!*p)
            return NULL;

        if (*p == '"')
        {
            start = ++p;

            while (*p && *p != '"')
                p++;

            if (*p != '"')
                return NULL;

            end = p++;
        }
        else
        {
            start = p;

            while (IsHTTPToken(*p))
                p++;

            if (p == start)
                return NULL;

            end = p;
        }

        while (IsLWS(*p))
            p++;

        if (*p && *p++ != ';')
            return NULL;

        *end = '\0';
        param->value = start;
    }

    return p;
}

/*
    Content-Type = "Content-Type" ":" media-type
    media-type = type "/" subtype *( ";" parameter )
    type = token
    subtype = token
*/
static int _HTTPParseContentType(
    PHIT_Headers* headers,
    HTTPBuf* buf,
    char* data,
    size_t size)
{
    char* p = data;

    /* Found */
    headers->contentType.found = 1;

    /* Get the 'type' */
    {
        char* start;
        char* end;

        while (IsLWS(*p))
            p++;

        start = p;

        while (IsHTTPToken(*p))
            p++;

        if (p == start)
            return -1;

        end = p;

        while (IsLWS(*p))
            p++;

        if (*p++ != '/')
            return -1;

        *end = '\0';
        headers->contentType.mediaType = start;
    }

    /* Get the 'subtype' */
    {
        char* start;
        char* end;

        while (IsLWS(*p))
            p++;

        start = p;

        while (IsHTTPToken(*p))
            p++;

        if (p == start)
            return -1;

        end = p;

        while (IsLWS(*p))
            p++;

        if (*p && *p++ != ';')
            return -1;

        *end = '\0';
        headers->contentType.mediaSubType = start;
    }

    /* Get each parameter */
    while (*p)
    {
        PHIT_HTTPParameter param;

        memset(&param, 0, sizeof(param));

        if (!(p = _ParseHTTPParameter(p, &param)))
            return -1;

        if (headers->contentType.nparameters ==
            PHIT_ARRAY_SIZE(buf->contentTypeParameters))
        {
            return -1;
        }

        buf->contentTypeParameters[headers->contentType.nparameters] = param;
        headers->contentType.parameters = buf->contentTypeParameters;
        headers->contentType.nparameters++;
    }

    return 0;
}

#if 0
const char* PHIT_HeadersFind(
    const PHIT_Headers* self,
    const char* name,
    size_t len)
{
    size_t i;

    for (i = 0; i < self->nheaders; i++)
    {
        const char* s = self->headers[i].name;

        if (strncasecmp(s, name, len) == 0 && s[len] == '\0')
            return self->headers[i].value;
    }

    /* Not found! */
    return NULL;
}
#endif

static int _ParseTokenList(
    char* data,
    const char** buf,
    size_t maxBufSize,
    size_t* bufSizeOut)
{
    char* p = data;

    for (;;)
    {
        char* start = p;

        /* Skip over LWS and commas */
        while (IsLWS(*p) || *p == ',')
            p++;

        /* Null terminate */
        if (p != start)
            *start = '\0';

        if (!*p)
            break;

        start = p;

        /* Skip over token characters */
        while (IsHTTPToken(*p))
            p++;

        if (p == start)
        {
            return -1;
        }

        /* Append trailer to list */
        {
            if (*bufSizeOut == maxBufSize)
                return -1;

            buf[(*bufSizeOut)++] = start;
        }

        if (!*p)
            break;
    }

    return 0;
}

/* From FastCGI:
   reqURI - REQUEST_URI == the full path:
   ex: REQUEST_URI     ='/rest-sync/v1/arg1'
   script - SCRIPT_FILENAME == munged URI with host:port + ResourcePath:
   ex: SCRIPT_FILENAME ='proxy:fcgi://localhost:4000/arg1'
 */
static void GetServiceRoot(
             PHIT_Headers* self, 
             char *reqURI, 
             char *script)
{
    // Construct serviceroot; Remove the trailing /; indicates possible segment data.
    char serviceroot[1024] = DEFAULT_SERVICE_ROOT;
    if (!reqURI || !(*reqURI) || !script || !(*script))
    {
      DEBUG_PRINTF("%s(): REQUEST_URI or SCRIPT_FILENAME is NULL\n", __FUNCTION__);
      goto SetServiceRoot;
    }

    // Get the resource portion of the script filename.
    char *resourcePath = strrchr(script, ':');
    if (!resourcePath) { goto SetServiceRoot; }

    // at the port #; find the next /; resource
    resourcePath = strchr(resourcePath, '/');
    if (!resourcePath) { goto SetServiceRoot; }

    // Put a NULL at the end of the request uri and we have serviceroot.
    strncpy(serviceroot, reqURI, sizeof(serviceroot));
    char *value = strstr(serviceroot, resourcePath);
    if (!value || !(*(value+1))) { goto SetServiceRoot; }

    *(value) = '\0'; // Don't return the trailing '/'.
    if (strlen(serviceroot) <= 1)
    { // no args specified; use REQUEST_URI directly.
      strncpy(serviceroot, reqURI, sizeof(serviceroot));
    }

  SetServiceRoot:
    // Set the Service Root
    setenv("SERVICE_ROOT", serviceroot, 1); 
    value = getenv("SERVICE_ROOT");
    self->ServiceRoot.found = 1;
    self->ServiceRoot.value = value;
    // SERVICE_ROOT ex: '/rest-sync/v1'
    DEBUG_PRINTF("%s(): SERVICE_ROOT    ='%s'\n", __FUNCTION__, value);

    value = getenv("REDFISH_VERSION");
    self->RedfishVersion.found = 1;    
    self->RedfishVersion.value = value;
    // REDFISH_VERSION ex: 0.94.0
    DEBUG_PRINTF("%s(): REDFISH_VERSION ='%s'\n", __FUNCTION__, value);
}

int FASTCGI_HeadersParse(
    PHIT_Headers* self,
    HTTPBuf* buf,
    char **env)
{
    /*HOTSPOT*/
    char* p = NULL;
    int foundContentType = 0;
    int foundTE = 0;
    char *reqURI = NULL;
    char *script = NULL;

    memset(self, 0, sizeof(*self));
    memset(buf, 0, sizeof(*buf));

    int i=0;
    for(i=0; env[i]; i++)
    {
        p=env[i];
        char c = toupper(*p);
        char *equal = strstr(p, "=");
        char *value = equal+1;
        size_t len=strlen(value);

        //DEBUG_PRINTF("%s(): i=%d, env=%s\n", __FUNCTION__, i, p);

        /* Match the header */
        // Translate Fast CGI headers to match PHIT ones...
        if (strncasecmp(p, STRLIT("HTTP_HOST=")) == 0)
        {
          DEBUG_PRINTF("HTTP_HOST\n");
          self->host.found = 1;
          self->host.value = value;
        }
        else if (strncasecmp(p, STRLIT("HTTP_USER_AGENT=")) == 0)
        {
          DEBUG_PRINTF("HTTP_USER_AGENT\n");
          self->userAgent.found = 1;
          self->userAgent.value = value;
        }
        else if (strncasecmp(p, STRLIT("REQUEST_URI=")) == 0)
        {
            reqURI = value;
        }
        else if (strncasecmp(p, STRLIT("SCRIPT_FILENAME=")) == 0)
        {
            script = value;
        }
        else if (!foundContentType && c == 'C' &&
            strncasecmp(p, STRLIT("Content_Type=")) == 0)
        {
            DEBUG_PRINTF("Content_type\n");
            if (_HTTPParseContentType(self, buf, value, len) != 0)
                return -1;

            foundContentType = 1;
        }
        else if (c == 'C' && strncasecmp(p, STRLIT("Content_Length=")) == 0)
        {
            DEBUG_PRINTF("Content_length\n");
            if (_HTTPParseContentLength(self, value, len) != 0)
            {
                return -1;
            }
        }
        else if (!foundTE &&
            ((p[0] == 'T' && p[1] == 'E' && p[2] == ':') ||
            Strncaseeq(p, STRLIT("TE:")) == 0))
        {
            DEBUG_PRINTF("TE\n");
            p = value;

            self->te.found = 1;

            for (; *p; p++)
            {
                if (StrncaseeqProbable(p, STRLIT("chunked")) == 0)
                {
                    self->te.chunked = 1;
                    p += STRN("chunked");
                }

                if (StrncaseeqProbable(p, STRLIT("trailers")) == 0)
                {
                    self->te.trailers = 1;
                    p += STRN("trailers");
                }
            }

            foundTE = 1;
        }
        else if (c == 'T' && strncasecmp(p, STRLIT("Transfer_Encoding=")) == 0)
        {
            D(printf("Transfer_Encoding\n");)
            p = value;

            self->transferEncoding.found = 1;

            for (; *p; p++)
            {
                if (strncasecmp(p, STRLIT("chunked")) == 0)
                {
                    self->transferEncoding.chunked = 1;
                    p += STRN("chunked");
                }
            }
        }
        else if (c == 'U' && strncasecmp(p, STRLIT("User_Agent=")) == 0)
        {
            D(printf("user agent\n");)
            self->userAgent.found = 1;
            self->userAgent.value = TrimLeadingAndTrailingLWS(value, len);
        }
        else if (c == 'H' && strncasecmp(p, STRLIT("Host=")) == 0)
        {
            D(printf("host\n");)
            self->host.found = 1;
            self->host.value = TrimLeadingAndTrailingLWS(value, len);
        }
        else if (c == 'A' && strncasecmp(p, STRLIT("Authorization=")) == 0)
        {
            D(printf("authorization\n");)
            self->authorization.found = 1;

            if (strncasecmp(value, STRLIT("Basic")) == 0)
            {
                p = value + STRN("Basic");
                self->authorization.type = PHIT_AUTHORIZATIONTYPE_BASIC;
                self->authorization.credentials = p;
            }
            else
            {
                self->authorization.type = PHIT_AUTHORIZATIONTYPE_UNKNOWN;
            }
        }
        else if (c == 'T' && strncasecmp(p, STRLIT("Trailer=")) == 0)
        {
            D(printf("trailer\n");)
            self->trailer.found = 1;

            if (_ParseTokenList(value, buf->trailersBuf, TRAILERS_BUFSIZE,
                &buf->trailersBufSize) == 0)
            {
                self->trailer.data = buf->trailersBuf;
                self->trailer.size = buf->trailersBufSize;
            }
            else
            {
                /* ATTN: ignore error */
            }
        }
        else
        {
            D(printf("generic\n");)
            /* Process a generic header */
            while (*p && *p != '=')
                p++;

            /* Expect a colon */
            if (*p != '=')
                return -1;

            /* Null terminate the header */
            *p = '\0';

            // Translate standard Fast CGI strings to their PHIT counterparts
            if (strncasecmp(env[i], STRLIT("HTTP_ACCEPT")) == 0)
            {
              D(printf("HTTP_ACCEPT\n");)
              _AppendHeader(buf->headersBuf, &buf->headersBufSize, "Accept", value);
            }
            else if (strncasecmp(env[i], STRLIT("HTTP_ACCEPT_LANGUAGE")) == 0)
            {
              D(printf("HTTP_ACCEPT_LANGUAGE\n");)
              _AppendHeader(buf->headersBuf, &buf->headersBufSize, "Accept-Language", value);
            }
            else if (strncasecmp(env[i], STRLIT("HTTP_ACCEPT_LANGUAGE")) == 0)
            {
              D(printf("HTTP_ACCEPT_ENCODING\n");)
              _AppendHeader(buf->headersBuf, &buf->headersBufSize, "Accept-Encoding", value);
            }
            else if (strncasecmp(env[i], STRLIT("HTTP_ACCEPT_LANGUAGE")) == 0)
            {
              D(printf("HTTP_CONNECTION\n");)
              _AppendHeader(buf->headersBuf, &buf->headersBufSize, "Connection", value);
            }
            else
            {
              _AppendHeader(buf->headersBuf, &buf->headersBufSize, env[i], value);
            }
        }

    }

    self->headers = buf->headersBuf;
    self->nheaders = buf->headersBufSize;

    GetServiceRoot(self, reqURI, script);

    return 0;
}

