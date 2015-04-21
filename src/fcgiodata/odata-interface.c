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

#define DEBUG_OUT(fd, prio, args...) syslog(prio, args)
#define DEBUG_PRINTF(args...) DEBUG_OUT(stderr, LOG_INFO, args)

int FASTCGI_HeadersParse(
    PHIT_Headers* self,
    HTTPBuf* buf,
    char **env);

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
        DEBUG_PRINTF("ERROR allocating memory to hold headers. Needed %zd bytes.\n", (num_http_headers + 1) * sizeof(char *));
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
    content=NULL;

out_free_envp_i:
    for(int i=0; envp[i]; i++)
        if (envp[i]) {
            free(envp[i]);
            envp[i] = NULL;
        }

out_free_envp:
    if(envp)
        free(envp);
    envp = NULL;

out:
    return ret;
}

// TODO: all messages we get in process_msg must have a response, so we don't
// hang up an fcgi frontent thread forever. All error paths must send a
// response unless the error is in the message send function.
int process_msg(zloop_t *loop, void *socket, Connection *c, zmsg_t *msg)
{
    int ret = -1;
    int z_ret = 0;

    // parse incoming message to extract http headers and content.
    if(zmsg_to_headers_and_content(msg, &(c->envp), &(c->content)) != 0) {
        DEBUG_PRINTF("ERROR parsing message to headers and conteng_to_headers_and_content\n");
        goto error_out;
    }

    // Parse HTTP Method
    char *methodStr = FCGX_GetParam("REQUEST_METHOD", c->envp);
    if(!methodStr)
        methodStr = "GET";
    PHIT_Method phit_method;
    ParseHTTPMethod(methodStr, &phit_method);
    DEBUG_PRINTF("method: %s(%d)\n", methodStr, (int)phit_method);

    // Parse Request URI
    char *requestUri = FCGX_GetParam("REQUEST_URI", c->envp);
    if (!requestUri) {
        DEBUG_PRINTF("ERROR requestUri was NULL\n");
        goto error_out;
    }

    char *remoteUser = FCGX_GetParam("REMOTE_USER", c->envp);
    if(remoteUser) {
        strncpy(c->user, remoteUser, USERNAME_SIZE-1);
    }
    DEBUG_PRINTF("Got REMOTE_USER: '%s'\n", c->user);

    // HTTP Headers to PHIT format
    HTTPBuf buf={};
    z_ret = FASTCGI_HeadersParse( &(c->headers), &buf, c->envp);
    if(z_ret) {
        DEBUG_PRINTF("ERROR parsing HTTP Headers\n");
        goto error_out;
    }

    // Finally call into ODATA. output is in the connection buffers.
    DEBUG_PRINTF("HandleRequest\n");
    __odataPlugin.base.HandleRequest(
        &__odataPlugin.base,
        (PHIT_Context *)(&c->context),
        phit_method,
        requestUri,
        &(c->headers),
        c->content,
        strlen(c->content));

    DEBUG_PRINTF("process msg: CHECK EOC: %d\n", c->context.postedEOC);

    ret = 0;
    goto out;

error_out:
    DEBUG_PRINTF("Handling error and cleaning up. FCGI_ConnectionDelete\n");
    FCGI_ConnectionDelete(&c);

out:
    return ret;
}

int server_task (zloop_t *loop, zmq_pollitem_t *item, void *arg)
{
    DEBUG_PRINTF("GOT message for server_task\n");
    zsock_t *reader = item->socket;
    zmsg_t *msg = zmsg_recv (reader);
    if(msg) {
        DEBUG_PRINTF("connectionnew\n");
        Connection* c = FCGI_ConnectionNew(loop, reader, msg);
        DEBUG_PRINTF("process_msg\n");
        process_msg(loop, reader, c, msg);
    }
    return 0;
}
