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
#ifndef lint
static const char rcsid[] = "$Id: echo.c,v 1.5 1999/07/28 00:29:37 roberts Exp $";
#endif /* not lint */

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

#ifdef _WIN32
#include <process.h>
#else
extern char **environ;
#endif

#define NO_FCGI_DEFINES
#include "fcgi_stdio.h"
#include "plugins/odata/odataplugin.h"
#include "fcgitest/context.h"
#include "fcgitest/connection.h"
#include "base/http.h"

static void PrintEnv(char *label, char **envp)
{
    FCGI_printf("%s:<br>\n<pre>\n", label);
    for ( ; *envp != NULL; envp++) {
        FCGI_printf("%s\n", *envp);
    }
    FCGI_printf("</pre><p>\n");
}

int main ()
{
    char **initialEnv = environ;
    int count = 0;

    // initialization
    __odataPlugin.base.Load(&__odataPlugin.base);


    while (FCGI_Accept() >= 0) {
        char *contentLengthStr = getenv("CONTENT_LENGTH");
        char *requestUri = getenv("REQUEST_URI");
        char *method = getenv("REQUEST_METHOD");
        int len;

        if (contentLengthStr != NULL) {
            len = strtol(contentLengthStr, NULL, 10);
        }
        else {
            len = 0;
        }

        Connection* c = ConnectionNew();
        char * content = NULL;
        PHIT_Method phit_method;
        ParseHTTPMethod(method, &phit_method);

        if (len>0){
            int i;
            content = calloc(len+1, 1);
            char *c = content;

            for (i = 0; i < len; i++) {
                if ((*c++ = getchar()) < 0) {
                    break;
		        }
            }
        }

        /* Content-Type: */
        PHIT_ContentTypeHeader contentType = {
            .found=0,
            .mediaType="",
            .mediaSubType="",
            .parameters=NULL,
            .nparameters=0,
            };

        /* Content-Length: */
        PHIT_ContentLengthHeader contentLength = {.found=1, .value=len};
    
        /* User-Agent: */
        PHIT_UserAgentHeader userAgent;
    
        /* Host: */
        PHIT_HostHeader host;
    
        /* Authorization: */
        PHIT_AuthorizationHeader authorization;
    
        /* TE: */
        PHIT_TEHeader te;
    
        /* Transfer-Encoding: */
        PHIT_TransferEncodingHeader transferEncoding;
    
        /* Trailer */
        PHIT_TrailerHeader trailer;
    
        /* Other headers */
        PHIT_Headers headers={{0},};
        size_t nheaders = 0;
    

        int count=0;
        for(; environ[count]; count++) /*empty*/ ;


#if 0
        __odataPlugin.base.HandleRequest(
            __odataPlugin.base,
            c->context,
            phit_method,
            requestUri,
            /*todo*/ headers,
            content,
            len);
#endif

        if(content) {
            free(content);
            content=NULL;
        }

    } /* while */

    // cleanup
    __odataPlugin.base.Unload(&__odataPlugin.base);

    return 0;
}
