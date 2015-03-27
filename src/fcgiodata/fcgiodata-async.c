#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>

#include <czmq.h>
#include <fcgiapp.h>
#include "plugins/odata/odataplugin.h"
#include "fcgi_connection.h"
#include "base/http.h"

#define THREAD_COUNT 20

#define DEBUG_OUT(fd, prio, args...) syslog(prio, args)
#define DEBUG_PRINTF(args...) DEBUG_OUT(stderr, LOG_INFO, args)

static int counts[THREAD_COUNT];
static int running;

void TESTME();
int FASTCGI_HeadersParse(
    PHIT_Headers* self,
    HTTPBuf* buf,
    char **env);

static void *handle_cgi_request(void *a)
{
    int rc, i; 
    long thread_id = (long)a;
    pid_t pid = getpid();
    FCGX_Request request;

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

        pthread_mutex_lock(&counts_mutex);
        ++counts[thread_id];
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
        if(headers)
            FCGX_FPrintF(request.out, "%s", headers);
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
    zframe_t *frame = zmsg_next(msg); // empty frame

    size_t num_frames = zmsg_size(msg);
    size_t num_http_headers = num_frames - 4;

    // allocate an array of chars to hold headers. This will look remarkably
    // similar to the fcgi envp stuff and so should be completely compatible.
    char **envp = calloc(num_http_headers + 1, sizeof(char *));

    // dup the strings into our new array
    for (int i=0; i<num_http_headers; i++)
    {
        frame = zmsg_next(msg);
        envp[i] = zframe_strdup(frame);
        DEBUG_PRINTF("COPY ENV: %s\n", envp[i]);
    }

    // extract HTTP content
    char *content = NULL;
    frame = zmsg_next(msg); // empty separator frame
    assert( zframe_size(frame) == 0 ); // damn well better be an empty frame
    frame = zmsg_next(msg); // the actual content frame
    content = zframe_strdup(frame); // convert it to a string with null termination

    Connection* c = FCGI_ConnectionNew();

    DEBUG_PRINTF("method parse. Content len(%d)\n", strlen(content));
    char *requestUri = FCGX_GetParam("REQUEST_URI", envp);
    DEBUG_PRINTF("request uri: '%s'", requestUri);
    char fallbackUri[] = "/INVALID_URI/";
    if (!requestUri)
        requestUri = fallbackUri;

    char *methodStr = FCGX_GetParam("REQUEST_METHOD", envp);
    DEBUG_PRINTF("method: '%s'", methodStr);
    if(!methodStr)
        methodStr = "GET";
    PHIT_Method phit_method;
    ParseHTTPMethod(methodStr, &phit_method);
    DEBUG_PRINTF("method: %d\n", (int)phit_method);

    DEBUG_PRINTF("FASTCGI_HeadersParse\n");
    PHIT_Headers headers={};
    HTTPBuf buf={};
    // IMPORTANT: no more FCGX_GetParam after this point!
    FASTCGI_HeadersParse( &headers, &buf, envp);

    DEBUG_PRINTF("request uri (again): '%s'", requestUri);
    DEBUG_PRINTF("HandleRequest\n");
    __odataPlugin.base.HandleRequest(
        &__odataPlugin.base,
        (PHIT_Context *)(&c->context),
        phit_method,
        requestUri,
        &headers,
        content,
        strlen(content));

    DEBUG_PRINTF("FCGI_ConnectionDelete\n");

    DEBUG_PRINTF("HEADERS: %s\nCONTENT: %s\n", c->wbuf.data, c->out.data);
    zframe_send(&identity, socket, ZFRAME_MORE | ZFRAME_REUSE );
    zstr_sendfm(socket, "%s", "");
    zstr_sendfm(socket, "%s", c->wbuf.data);
    zstr_sendf(socket, "%s", c->out.data);

    FCGI_ConnectionDelete(c);

    for(int i=0; envp[i]; i++) free(envp[i]);
    free(envp);
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

    // initialization of odata
    DEBUG_PRINTF("load\n");
    __odataPlugin.base.Load(&__odataPlugin.base);
    DEBUG_PRINTF("load done\n");

    running=1;

    FCGX_Init();

    for (i = 0; i < THREAD_COUNT; i++)
        pthread_create(&id[i], NULL, handle_cgi_request, (void*)i);

    server_task(0);

    return 0;
}
