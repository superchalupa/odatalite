#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>

#include "czmq.h"
#include "fcgiapp.h"

#define THREAD_COUNT 20

// zmq compat 1.4->3.0
#if CZMQ_VERSION_MAJOR < 3
#define zstr_send(dest, string) zstr_sendf(dest, "%s", string)
#define zstr_sendm(dest, string) zstr_sendfm(dest, "%s", string)
#endif

int running;

static void *handle_request(void *a)
{
    long rc, thread_id = (long)a;
    pid_t pid = getpid();
    FCGX_Request request;
    char *server_name;

    zctx_t *ctx = zctx_new ();
    void *cgiserver = zsocket_new (ctx, ZMQ_REQ);

    //  Set random identity to make tracing easier
    char identity [10];
    sprintf (identity, "fcgi-listener-%ld", thread_id);
    zsocket_set_identity (cgiserver, identity);
    zsocket_connect (cgiserver, "tcp://localhost:5570");

    FCGX_InitRequest(&request, 0, 0);

    printf( "Thread %ld (%d) started\n", thread_id, pid);

    for (;;)
    {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;

        /* Some platforms require accept() serialization, some don't.. */
        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

        if (rc < 0)
            break;

        int len;

        //PrintEnv("REQUEST", environ);
        char *contentLengthStr = FCGX_GetParam("CONTENT_LENGTH", request.envp);
        if (contentLengthStr != NULL) {
            len = strtol(contentLengthStr, NULL, 10);
        }
        else {
            len = 0;
        }

        server_name = FCGX_GetParam("SERVER_NAME", request.envp);

        for(char **e = request.envp; e; e++)
            zstr_sendfm(cgiserver, "%s", *e);

        zstr_sendfm(cgiserver, "");

        if (len>0){
            int i;
            char *content = calloc(len+1, 1);
            char *c = content;

            for (i = 0; i < len; i++) {
                if ((*c++ = getchar()) < 0) {
                    break;
		        }
            }

            zstr_sendf(cgiserver, "%s", content);
        }

        FCGX_FPrintF(request.out,
            "Content-type: text/html\r\n"
            "\r\n"
            "Hello world, from thread %ld (%d) on server %s",
            thread_id, pid, server_name ? server_name : "?");

        sleep(2);
        FCGX_Finish_r(&request);
    }

    printf( "Thread %ld (%d) exiting\n", thread_id, pid);
    zctx_destroy (&ctx);
    running=0;

    return NULL;
}

void *server_task (void *args)
{
    //  cgiserver socket talks to clients over TCP
    zctx_t *ctx = zctx_new ();
    void *cgiserver = zsocket_new (ctx, ZMQ_ROUTER);
    zsocket_bind (cgiserver, "tcp://*:5570");
    printf("Starting up server task\n");

    zmq_pollitem_t items [] = { { cgiserver, 0, ZMQ_POLLIN, 0 } };
    while (running) {
        zmq_poll (items, 1, 100 * ZMQ_POLL_MSEC);
            if (items [0].revents & ZMQ_POLLIN) {
                zmsg_t *msg = zmsg_recv (cgiserver);
                zframe_t *identity = zmsg_pop (msg);
                zframe_print (zmsg_last (msg), identity);
                //zframe_t *content = zmsg_pop (msg);
                zmsg_destroy (&msg);
            }
    }

    zctx_destroy (&ctx);
    printf("Shut down server task\n");
    return NULL;
}

int main(void)
{
    printf("Hello world.\n");
    FCGX_Init();
    printf("Init complete.\n");

    for (long i = 0; i < THREAD_COUNT; i++) {
        printf("Creating thread %ld\n", i);
        zthread_new(handle_request, (void*)i);
    }

    printf("Creating thread 0\n");
    running=1;
    server_task(0);

    return 0;
}
