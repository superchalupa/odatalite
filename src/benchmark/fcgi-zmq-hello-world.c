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

#include <czmq.h>
#include <fcgiapp.h>

#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#include "config.h"

#ifndef FCGI_ACCEPT_HANDLER_THREAD_COUNT
#define FCGI_ACCEPT_HANDLER_THREAD_COUNT 20
#endif

#define DEBUG_OUT(fd, prio, args...) syslog(prio, args)
#define DEBUG_PRINTF(args...) DEBUG_OUT(stderr, LOG_INFO, args)

#ifdef HAVE_SYSTEMD
#include "systemd/sd-daemon.h"
#else
int sd_notify(int unset_environment, const char *state) {return 0;}
#endif

int server_task (zloop_t *loop, zmq_pollitem_t *item, void *arg);

struct cgi_thread_parameters
{
    zctx_t *ctx;
    pthread_t thread_id;
};

static void *handle_cgi_request(void *a)
{
    int rc;
    pthread_t thread_id = ((struct cgi_thread_parameters *)a)->thread_id;
    zctx_t *ctx = ((struct cgi_thread_parameters *)a)->ctx;
    FCGX_Request request;

    // Allow thread to be cancelled at any time
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    // create ZMQ context for IPC to our server task
    void *cgiserver = zsocket_new (ctx, ZMQ_REQ);

    //  Set random identity to make tracing easier
    char identity [30]={0};
    sprintf (identity, "fcgi-listener-%ld", thread_id);
    zsocket_set_identity (cgiserver, identity);
    zsocket_connect (cgiserver, "inproc://cgi-thread");

    FCGX_InitRequest(&request, 0, 0);

    for (;;)
    {
        rc = FCGX_Accept_r(&request);

        if (rc < 0)
            break;

        int len = 0;
        char *contentLengthStr = FCGX_GetParam("CONTENT_LENGTH", request.envp);
        if (contentLengthStr != NULL)
            len = strtol(contentLengthStr, NULL, 10);

        // Send all HTTP HEADERS over to server process. One header per FRAME.
        for(int i=0; request.envp[i]; i++)
        {
            char *p=request.envp[i];
            zstr_sendfm(cgiserver, "%s", p);
        }

        // NULL Frame separates headers from content
        zstr_sendfm(cgiserver, "%s", "");

        // Read Content from Apache over FCGI connection
        // TODO: this isn't the best way to do this: it does one char at a time and doesn't do EOF checking.
        char *content = NULL;
        if (len>0){
            content = calloc(len+1, 1);
            char *c = content;

            for (int i = 0; i < len; i++) {
                if ((*c++ = getchar()) < 0) {
                    break;
		        }
            }
            // Send HTTP CONTENT to server
            zstr_sendf(cgiserver, "%s", content);
            if(content)
                free(content);
        } else {
            // no content
            zstr_sendf(cgiserver, "%s", "");
        }

        //**********************************
        // RESPONSE
        //
        // print out the response we got back from the server

        char *headers = NULL;
        content = NULL;
        zstr_recvx(cgiserver, &headers, &content);
        if(headers)
        {
            FCGX_FPrintF(request.out, "%s", headers);
            if(content)
                FCGX_FPrintF(request.out, "%s", content);
        }
        zstr_free(&headers);
        zstr_free(&content);

        FCGX_Finish_r(&request);
        FCGX_Free(&request, 1);
    }

    DEBUG_PRINTF("ACCEPT THREAD ENDED");

    zsocket_destroy(ctx, &cgiserver);

    return NULL;
}


int process_msg(zloop_t *loop, void *socket, zmsg_t *msg)
{
    // return from timer: 0 == keep running timer, -1 == stop timer
    int ret = 0;

    int z_ret = 0;

    // Create response message
    zmsg_t *response = zmsg_new();
    if(!response) {
        DEBUG_PRINTF("Error creating response message\n");
        goto error_sending;
    }

    // IDENTITY frame (dup existing because the original message still owns it)
    zframe_t *return_identity = zmsg_first (msg); // client identity frame
    zframe_t *response_dest = zframe_dup(return_identity);
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
    const char *HEADER = "Content-type: text/html\r\n";
    z_ret = zmsg_addmem( response, HEADER, strlen(HEADER));
    if(z_ret){
        DEBUG_PRINTF("Error appending http response headers\n");
        goto error_sending;
    }

    // CONTENT frame
    const char *CONTENT = "HELLO WORLD";
    z_ret = zmsg_addmem( response, CONTENT, strlen(CONTENT));
    if(z_ret){
        DEBUG_PRINTF("Error appending http response content\n");
        goto error_sending;
    }

    // Actually send response message
    z_ret = zmsg_send(&response, socket);
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

    return ret;
}

int watchdog_ping (zloop_t *loop, int timer_id, void *arg)
{
    (void)timer_id;
    sd_notify(0, "WATCHDOG=1");
    return 0;
}

int server_task (zloop_t *loop, zmq_pollitem_t *item, void *arg)
{
    zsock_t *reader = item->socket;
    zmsg_t *msg = zmsg_recv (reader);
    if(msg) {
        process_msg(loop, reader, msg);
    }
    return 0;
}
int main(void)
{
    struct cgi_thread_parameters thread_params[FCGI_ACCEPT_HANDLER_THREAD_COUNT] = {};

    DEBUG_PRINTF("Async odata fastcgi server starting up.\n");

    // CZMQ context
    zctx_t *ctx = zctx_new ();

    // let all the zmq printing functions use syslog since it's not safe to use printf()
    zsys_set_logsystem(1);

    // Initialize FCGI and start handler threads
    FCGX_Init();
    pthread_attr_t attr;
    int s = pthread_attr_init(&attr);
    if (s != 0) {
        DEBUG_PRINTF("pthread_attr_init failed: %d", s);
        exit(1);
    }
    pthread_attr_setdetachstate(&attr, 1);

    for (long i = 0; i < FCGI_ACCEPT_HANDLER_THREAD_COUNT; i++) {
        // pass in our context to the fcgi thread so that we can use inproc:// between threads
        thread_params[i].ctx = ctx;
        pthread_create(&thread_params[i].thread_id, &attr, handle_cgi_request, (void*)(&thread_params[i]));
    }

    pthread_attr_destroy(&attr);

    // Create our event reactor
    zloop_t *reactor = zloop_new ();
    assert (reactor);
    //zloop_set_verbose (reactor, 1);

    // Listen for incoming requests on inproc socket, call handler on incoming messages
    void *cgiserver = zsocket_new (ctx, ZMQ_ROUTER);
    zsocket_bind (cgiserver, "inproc://cgi-thread");
    zmq_pollitem_t poller = { cgiserver, 0, ZMQ_POLLIN };
    zloop_poller (reactor, &poller, server_task, NULL);

    // register watchdog timer
    char *wd_usec_str = getenv("WATCHDOG_USEC");
    if(wd_usec_str) {
        int wd_interval_ms = strtoul(wd_usec_str, NULL, 0) / 1000 / 3;
        DEBUG_PRINTF("  Setting up systemd process watchdog handler every %dms\n", wd_interval_ms);
        if (wd_interval_ms > 0) {
            // send in three watchdog pings per watchdog timeout period. zloop expects ms timeout.
            zloop_timer (reactor, wd_interval_ms, 0, watchdog_ping, NULL);
        }
    }

    DEBUG_PRINTF("Async odata fastcgi server entering request handling loop.\n");
    // don't notify systemd about startup complete until we are about to enter processing loop
    sd_notify(0, "READY=1");
    sd_notify(0, "STATUS=Processing requests.");
    zloop_start(reactor);  // never returns until SIGINT
    sd_notify(0, "STATUS=Exiting due to interrupt signal.");

    DEBUG_PRINTF("Interrupted. Exiting...\n");

    zloop_destroy(&reactor);
    zsocket_destroy(ctx, cgiserver);
    zctx_destroy (&ctx);

    exit(0);
}
