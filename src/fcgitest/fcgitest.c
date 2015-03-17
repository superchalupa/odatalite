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
#include "src/plugins/odata/odataplugin.h"

static void PrintEnv(char *label, char **envp)
{
    FCGI_printf("%s:<br>\n<pre>\n", label);
    for ( ; *envp != NULL; envp++) {
        FCGI_printf("%s\n", *envp);
    }
    FCGI_printf("</pre><p>\n");
}

/* Post HTTP status line */
void PostStatusLine(
        PHIT_Context* context,
        PHIT_StatusCode statusCode,
        const char* statusMsg)
{}

/* Post HTTP header field */
void PostHeader(
        PHIT_Context* context,
        const char* name,
        const char* value)
{}

/* Post HTTP header field */
void PostHeaderUL(
        PHIT_Context* context,
        const char* name,
        unsigned long value)
{}

/* Post HTTP header field */
void PostTrailerField(
        PHIT_Context* context,
        const char* name,
        const char* value)
{}

/* Post end of headers */
void PostEOH(
        PHIT_Context* context)
{}

/* Post HTTP content */
void PostContent(
        PHIT_Context* context,
        const char* data,
        size_t size)
{}

/* Post end of content */
void PostEOC(
        PHIT_Context* context)
{}

/* Post an error message */
void PostError(
        PHIT_Context* context,
        PHIT_StatusCode statusCode,
        const char* statusMsg,
        const char* detail)
{}

/* Set client data */
void* SetPluginData(
        PHIT_Context* context,
        void* pluginData)
{}

/* Get client data */
void* GetPluginData(
        PHIT_Context* context)
{}

/* Get option */
int GetOption(
        const PHIT_Context* context,
        int option,
        void* value,
        size_t valueSize)
{}

PHIT_Context fastcgi_context = {
    .PostStatusLine = &PostStatusLine,
    .PostHeader = &PostHeader,
    .PostHeaderUL = &PostHeaderUL,
    .PostTrailerField = &PostTrailerField,
    .PostEOH = &PostEOH,
    .PostContent = &PostContent,
    .PostEOC = &PostEOC,
    .PostError = &PostError,
    .SetPluginData = &SetPluginData,
    .GetPluginData = &GetPluginData,
    .GetOption = &GetOption,
};

void (*HandleRequest)(
        PHIT_Plugin* plugin,
        PHIT_Context* context,
        PHIT_Method method,
        const char* requestURI,
        const PHIT_Headers* headers,
        const char* content,
        size_t contentLength);

int main ()
{
    char **initialEnv = environ;
    int count = 0;

    // initialization
    __odataPlugin.base.Load(&__odataPlugin.base);


    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

	FCGI_printf("Content-type: text/html\r\n"
	    "\r\n"
	    "<title>FastCGI echo</title>"
	    "<h1>FastCGI echo</h1>\n"
            "Request number %d,  Process ID: %d<p>\n", ++count, getpid());

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }

        if (len <= 0) {
	    FCGI_printf("No data from standard input.<p>\n");
        }
        else {
            int i, ch;

	    FCGI_printf("Standard input:<br>\n<pre>\n");
            for (i = 0; i < len; i++) {
                if ((ch = getchar()) < 0) {
                    FCGI_printf("Error: Not enough bytes received on standard input<p>\n");
                    break;
		}
                putchar(ch);
            }
            FCGI_printf("\n</pre><p>\n");
        }

        PrintEnv("Request environment", environ);
        PrintEnv("Initial environment", initialEnv);
    } /* while */

    // cleanup
    __odataPlugin.base.Unload(&__odataPlugin.base);

    return 0;
}
