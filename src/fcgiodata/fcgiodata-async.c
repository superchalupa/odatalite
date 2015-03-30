#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>

#include <czmq.h>
#include <fcgiapp.h>
#include "plugins/odata/odataplugin.h"
#include "connection-fcgi.h"
#include "base/http.h"

#define THREAD_COUNT 20

#define DEBUG_OUT(fd, prio, args...) syslog(prio, args)
#define DEBUG_PRINTF(args...) DEBUG_OUT(stderr, LOG_INFO, args)

static int counts[THREAD_COUNT];

int FASTCGI_HeadersParse(
    PHIT_Headers* self,
    HTTPBuf* buf,
    char **env);

static void *handle_cgi_request(void *a)
{
    int rc;
    long thread_id = (long)a;
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

int zmsg_to_headers_and_content(zmsg_t *msg_in, char ***envp_out, char **content_out)
{
    int ret = -1;
    zframe_t *frame = zmsg_first (msg_in); // client identity frame
    frame = zmsg_next(msg_in); // empty frame
    assert( zframe_size(frame) == 0 ); // make sure it's empty

    // identity frame + null frame at the beginning and null frame + content at the end.
    // the rest are all headers
    size_t num_http_headers = zmsg_size(msg_in) - 4;

    // allocate an array of chars to hold headers. This will look remarkably
    // similar to the fcgi envp stuff and so should be completely compatible.
    char **envp = calloc(num_http_headers + 1, sizeof(char *));
    if (!envp) {
        DEBUG_PRINTF("ERROR allocating memory to hold headers. Needed %d bytes.\n", (num_http_headers + 1) * sizeof(char *));
        goto out_free_envp;
    }

    // dup the strings into our new array
    for (int i=0; i<num_http_headers; i++)
    {
        frame = zmsg_next(msg_in);
        envp[i] = zframe_strdup(frame);
        if(!envp[i]) {
            DEBUG_PRINTF("ERROR allocating memory to hold env pointer.\n");
            goto out_free_envp_i;
        }
    }

    // extract HTTP content
    char *content = NULL;
    frame = zmsg_next(msg_in); // empty separator frame
    assert( zframe_size(frame) == 0 ); // damn well better be an empty frame
    frame = zmsg_next(msg_in); // the actual content frame
    content = zframe_strdup(frame); // convert it to a string with null termination
    if(!content) {
        DEBUG_PRINTF("ERROR allocating memory to hold content pointer.\n");
        goto out_free_content;
    }

    *envp_out = envp;
    *content_out = content;
    ret = 0;
    goto out;

out_free_content:
    if(content)
        free(content);

out_free_envp_i:
    for(int i=0; envp[i]; i++)
        if (envp[i])
            free(envp[i]);

out_free_envp:
    if(envp)
        free(envp);

out:
    return ret;
}

int process_msg(void *socket, Connection *c, zmsg_t *msg)
{
    int ret = -1;
    int z_ret = 0;

    zframe_t *identity = zmsg_first (msg); // client identity frame

    // parse incoming message to extract http headers and content.
    char **envp = NULL, *content = NULL;
    if(zmsg_to_headers_and_content(msg, &envp, &content) != 0) {
        DEBUG_PRINTF("ERROR parsing message to headers and conteng_to_headers_and_content\n");
        goto out_free;
    }

    // Parse HTTP Method
    char *methodStr = FCGX_GetParam("REQUEST_METHOD", envp);
    if(!methodStr)
        methodStr = "GET";
    PHIT_Method phit_method;
    ParseHTTPMethod(methodStr, &phit_method);
    DEBUG_PRINTF("method: %s(%d)\n", methodStr, (int)phit_method);

    // Parse Request URI
    char *requestUri = FCGX_GetParam("REQUEST_URI", envp);
    if (!requestUri) {
        DEBUG_PRINTF("ERROR requestUri was NULL\n");
        goto out_free;
    }

    // HTTP Headers to PHIT format
    PHIT_Headers headers={};
    HTTPBuf buf={};
    z_ret = FASTCGI_HeadersParse( &headers, &buf, envp);
    if(z_ret) {
        DEBUG_PRINTF("ERROR parsing HTTP Headers\n");
        goto out_free;
    }

    // Finally call into ODATA. output is in the connection buffers.
    DEBUG_PRINTF("HandleRequest\n");
    __odataPlugin.base.HandleRequest(
        &__odataPlugin.base,
        (PHIT_Context *)(&c->context),
        phit_method,
        requestUri,
        &headers,
        content,
        strlen(content));

    // Create response message
    zmsg_t *response = zmsg_new();
    if(!response)
        goto error_sending;

    // IDENTITY frame (dup existing because the original message still owns it)
    zframe_t *response_dest = zframe_dup(identity);
    if(!response_dest)
        goto error_sending;

    z_ret = zmsg_append( response, &response_dest );
    if(z_ret)
        goto error_sending;

    // NULL separator frame
    z_ret = zmsg_addstr( response, "" );
    if(z_ret)
        goto error_sending;

    // HEADER frame
    z_ret = zmsg_addmem( response, c->wbuf.data, c->wbuf.size );
    if(z_ret)
        goto error_sending;

    // CONTENT frame
    z_ret = zmsg_addmem( response, c->out.data, c->out.size );
    if(z_ret)
        goto error_sending;

    // Actually send response message
    z_ret = zmsg_send(&response, socket);
    if(z_ret)
        goto error_sending;

    ret = 0;
    goto out_free;

error_sending:
    // normally zmq will take over message and destroy it, however if send
    // fails or we fail to add all our frames, we have to dispose of it
    // ourselves.
    DEBUG_PRINTF("ERROR sending message, destroying it.\n");
    zmsg_destroy(&response);

out_free:
    // free everything in reverse order of allocation
    if(content) {
        free(content);
        content = NULL;
    }

    for(int i=0; envp[i]; i++) {
        if (envp[i])
            free(envp[i]);
        envp[i] = NULL;
    }

    if (envp)
        free(envp);
    envp=NULL;

    DEBUG_PRINTF("Processed message. Ret=%d\n", ret);
    return ret;
}

int server_task (zloop_t *loop, zmq_pollitem_t *item, void *arg)
{
    zsock_t *reader = item->socket;
    zmsg_t *msg = zmsg_recv (reader);
    if(msg) {
        Connection* c = FCGI_ConnectionNew();
        process_msg(reader, c, msg);
        DEBUG_PRINTF("FCGI_ConnectionDelete\n");
        FCGI_ConnectionDelete(c);
        zmsg_destroy (&msg);
    }
    return 0;
}


int main(void)
{
    int i;
    pthread_t id[THREAD_COUNT];

    DEBUG_PRINTF("Async odata fastcgi server starting up.\n");

    // initialization of odata
    __odataPlugin.base.Load(&__odataPlugin.base);

    // Initialize FCGI and start handler threads
    FCGX_Init();
    for (i = 0; i < THREAD_COUNT; i++)
        pthread_create(&id[i], NULL, handle_cgi_request, (void*)i);

    // CZMQ context
    zctx_t *ctx = zctx_new ();

    // Listen for incoming requests on localhost:5570
    zsock_t *cgiserver = zsocket_new (ctx, ZMQ_ROUTER);
    zsocket_bind (cgiserver, "tcp://lo:5570");

    // let all the zmq printing functions use syslog since it's not safe to use printf()
    zsys_set_logsystem(1);

    // Create our event reactor
    DEBUG_PRINTF("   Create reactor\n");
    zloop_t *reactor = zloop_new ();
    assert (reactor);
    zloop_set_verbose (reactor, 1);

    DEBUG_PRINTF("   Create reader\n");
    zmq_pollitem_t poller = { cgiserver, 0, ZMQ_POLLIN };
    zloop_poller (reactor, &poller, server_task, NULL);

    DEBUG_PRINTF("   Start reactor\n");
    zloop_start(reactor);

    zloop_destroy(&reactor);
    zsock_destroy(&cgiserver);
    zctx_destroy (&ctx);

    return 0;
}
