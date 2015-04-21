#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>

#include <czmq.h>
#include <fcgiapp.h>
#include "plugins/odata/odataplugin.h"
#include "fcgiodata/connection.h"
#include "base/http.h"
#include "base/path.h"

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
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

        /* Some platforms require accept() serialization, some don't.. */
        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

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
        // TODO: not sure if this is robust, check docs for best way... probably dont want to do char at a time
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
    }

    zctx_destroy (&ctx);

    return NULL;
}


int watchdog_ping (zloop_t *loop, int timer_id, void *arg)
{
    (void)timer_id;
    DEBUG_PRINTF("Ping watchdog\n");
    sd_notify(0, "WATCHDOG=1");
    return 0;
}

int main(void)
{
    struct cgi_thread_parameters thread_params[FCGI_ACCEPT_HANDLER_THREAD_COUNT] = {};

    DEBUG_PRINTF("Async odata fastcgi server starting up.\n");

    // CZMQ context
    zctx_t *ctx = zctx_new ();

    // initialization of odata
    __odataPlugin.base.Load(&__odataPlugin.base);

    // Initialize FCGI and start handler threads
    FCGX_Init();
    for (long i = 0; i < FCGI_ACCEPT_HANDLER_THREAD_COUNT; i++) {
        // pass in our context to the fcgi thread so that we can use inproc:// between threads
        thread_params[i].ctx = ctx;
        pthread_create(&thread_params[i].thread_id, NULL, handle_cgi_request, (void*)(&thread_params[i]));
    }

    // let all the zmq printing functions use syslog since it's not safe to use printf()
    zsys_set_logsystem(1);

    // Create our event reactor
    DEBUG_PRINTF("   Create reactor\n");
    zloop_t *reactor = zloop_new ();
    assert (reactor);
    zloop_set_verbose (reactor, 1);

    // Listen for incoming requests on localhost:5570
    zsock_t *cgiserver = zsocket_new (ctx, ZMQ_ROUTER);
    zsocket_bind (cgiserver, "inproc://cgi-thread");

    // create handler for incoming messages
    DEBUG_PRINTF("   Create reader\n");
    zmq_pollitem_t poller = { cgiserver, 0, ZMQ_POLLIN };
    zloop_poller (reactor, &poller, server_task, NULL);

    // register watchdog timer
    char *wd_usec_str = getenv("WATCHDOG_USEC");
    if(wd_usec_str) {
        int wd_timeout_us = strtoul(wd_usec_str, NULL, 0);
        DEBUG_PRINTF("   watchdog interval: %dms\n", (wd_timeout_us /1000) / 3);
        if (wd_timeout_us > 0) {
            // send in three watchdog pings per watchdog timeout period. zloop expects ms timeout.
            zloop_timer (reactor, (wd_timeout_us /1000) / 3, 0, watchdog_ping, NULL);
        }
    }

    DEBUG_PRINTF("   Start reactor\n");
    sd_notify(0, "READY=1");
    zloop_start(reactor);  // never returns until SIGINT

    zloop_destroy(&reactor);
    zsock_destroy(&cgiserver);
    zctx_destroy (&ctx);

    return 0;
}
