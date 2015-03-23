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

int FASTCGI_HeadersParse(
    PHIT_Headers* self,
    HTTPBuf* buf,
    char **env);

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
    //char **initialEnv = environ;

    // initialization
    __odataPlugin.base.Load(&__odataPlugin.base);


    while (FCGI_Accept() >= 0) {
        char *contentLengthStr = getenv("CONTENT_LENGTH");
        char *requestUri = getenv("REQUEST_URI");
        char *methodStr = getenv("REQUEST_METHOD");
        int len;

        PrintEnv("REQUEST", environ);
        FCGI_printf("hello world 1\n");

        if (contentLengthStr != NULL) {
            len = strtol(contentLengthStr, NULL, 10);
        }
        else {
            len = 0;
        }

        FCGI_printf("hello world 2\n");

        Connection* c = ConnectionNew();
        (void)c;

        FCGI_printf("hello world 3 '%s'\n", methodStr);
        char * content = NULL;
        PHIT_Method phit_method;
        const char * ret = ParseHTTPMethod(methodStr, &phit_method);

        FCGI_printf("hello world 4: method '%d'\n", phit_method);
        FCGI_printf("hello world 4: ret '%s'\n", ret);

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

        FCGI_printf("hello world 5: parsing headers\n");
        PHIT_Headers headers={};
        HTTPBuf buf={};
        FASTCGI_HeadersParse( &headers, &buf, environ);
        FCGI_printf("hello world 5: parsed headers\n");

        PHIT_HeadersDump(&headers, 1);

        __odataPlugin.base.HandleRequest(
            &__odataPlugin.base,
            (PHIT_Context *)(&c->context),
            phit_method,
            requestUri,
            &headers,
            content,
            len);

        if(content) {
            free(content);
            content=NULL;
        }

    } /* while */

    // cleanup
    __odataPlugin.base.Unload(&__odataPlugin.base);

    return 0;
}
