#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>

#include <czmq.h>
#include <fcgiapp.h>

#define THREAD_COUNT 20

#define DEBUG_OUT(fd, prio, args...) syslog(prio, args)
#define DEBUG_PRINTF(args...) DEBUG_OUT(stderr, LOG_INFO, args)

static int counts[THREAD_COUNT];
static int running;

static void *handle_cgi_request(void *a)
{
    int rc, i; 
    long thread_id = (long)a;
    pid_t pid = getpid();
    FCGX_Request request;
    char *server_name;

    // create ZMQ context for IPC to our server task
    zctx_t *ctx = zctx_new ();
    void *cgiserver = zsocket_new (ctx, ZMQ_REQ);

    //  Set random identity to make tracing easier
    char identity [30]={0};
    sprintf (identity, "fcgi-listener-%ld", thread_id);
    zsocket_set_identity (cgiserver, identity);
    zsocket_connect (cgiserver, "tcp://localhost:5570");

    FCGX_InitRequest(&request, 0, 0);

    for (;;)
    {
        static pthread_mutex_t accept_mutex = PTHREAD_MUTEX_INITIALIZER;
        static pthread_mutex_t counts_mutex = PTHREAD_MUTEX_INITIALIZER;

        /* Some platforms require accept() serialization, some don't.. */
        pthread_mutex_lock(&accept_mutex);
        rc = FCGX_Accept_r(&request);
        pthread_mutex_unlock(&accept_mutex);

        if (rc < 0)
            break;

        server_name = FCGX_GetParam("SERVER_NAME", request.envp);

        // Output some test data for now, until odataplugin is hooked in
        FCGX_FPrintF(request.out,
            "Content-type: text/html\r\n"
            "\r\n"
            "<title>FastCGI Hello! (multi-threaded C, fcgiapp library)</title>"
            "<h1>FastCGI Hello! (multi-threaded C, fcgiapp library)</h1>"
            "Thread %ld, Process %ld<p>"
            "Request counts for %d threads running on host <i>%s</i><p><code>",
            thread_id, pid, THREAD_COUNT, server_name ? server_name : "?");

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
        zstr_sendf(cgiserver, "");

        // Read Content from Apache over FCGI connection
        // TODO: not sure if this is robust, check docs for best way... probably dont want to do char at a time
        char *content = {0};
        if (len>0){
            content = calloc(len+1, 1);
            char *c = content;

            for (int i = 0; i < len; i++) {
                if ((*c++ = getchar()) < 0) {
                    break;
		        }
            }
        }

        // Send HTTP CONTENT to server
        zstr_sendf(cgiserver, "%s", content);
        if(content)
            free(content);

        pthread_mutex_lock(&counts_mutex);
        ++counts[thread_id];
        for (i = 0; i < THREAD_COUNT; i++)
            FCGX_FPrintF(request.out, "%5d " , counts[i]);
        pthread_mutex_unlock(&counts_mutex);

        //**********************************
        // RESPONSE
        //
        // print out the response we got back from the server

        char *headers = NULL;
        content = NULL;
        zstr_recvx(cgiserver, &headers, &content);
        DEBUG_PRINTF("GOT HEADERS: %s\n", headers);
        DEBUG_PRINTF("GOT CONTENT: %s\n", content);
        FCGX_FPrintF(request.out, "\n");
        if(headers)
            FCGX_FPrintF(request.out, "%s\r\n\r\n", headers);
        if(content)
            FCGX_FPrintF(request.out, "%s", content);
        zstr_free(&headers);
        zstr_free(&content);

        FCGX_Finish_r(&request);
    }

    zctx_destroy (&ctx);
    running=0;

    return NULL;
}

void process_msg(void *socket, zmsg_t *msg)
{
    zframe_t *identity = zmsg_first (msg);
    zframe_t *frame = identity;
    do {
        char *foo = zframe_strdup(frame);
        free(foo);
    } while( (frame = zmsg_next(msg)) != 0 );

    zframe_send(&identity, socket, ZFRAME_MORE | ZFRAME_REUSE );
    zstr_sendfm(socket, "");
    zstr_sendfm(socket, "headers");
    zstr_sendf(socket, "content");
}

void *server_task (void *args)
{
    //  cgiserver socket talks to clients over TCP
    zctx_t *ctx = zctx_new ();
    void *cgiserver = zsocket_new (ctx, ZMQ_ROUTER);
    zsocket_bind (cgiserver, "tcp://*:5570");
    DEBUG_PRINTF("Starting up server task\n");
    zsys_set_logsystem(1);

    zmq_pollitem_t items [] = { { cgiserver, 0, ZMQ_POLLIN, 0 } };
    while (running) {
        zmq_poll (items, 1, 100 * ZMQ_POLL_MSEC);
            if (items [0].revents & ZMQ_POLLIN) {
                zmsg_t *msg = zmsg_recv (cgiserver);
                process_msg(cgiserver, msg);
                zmsg_destroy (&msg);
           }
    }

    zctx_destroy (&ctx);
    DEBUG_PRINTF("Shut down server task\n");
    return NULL;
}


int main(void)
{
    int i;
    pthread_t id[THREAD_COUNT];

    DEBUG_OUT(stderr, LOG_INFO, "hello world\n");
    running=1;

    FCGX_Init();

    for (i = 0; i < THREAD_COUNT; i++)
        pthread_create(&id[i], NULL, handle_cgi_request, (void*)i);

    server_task(0);

    return 0;
}
