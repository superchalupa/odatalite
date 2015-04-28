/*
 * fcgiauth.c --
 *
 *	FastCGI Authorization and Authentication
 *
 *
 *
 */

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include "fcgi_config.h"
#include "fcgi_stdio.h"

#define DEBUG_OUT(fd, prio, args...) syslog(prio, args)
#define DEBUG_PRINT_ERR(args...) DEBUG_OUT(stderr, LOG_ERR, args)
#define DEBUG_PRINT_INFO(args...) DEBUG_OUT(stderr, LOG_INFO, args)
#define DEBUG_PRINT_DEBUG(args...) DEBUG_OUT(stderr, LOG_DEBUG, args)


#define GETSTR(X) X?X:""

struct authContext
{
    const char *user;
    const char *password;

	const char *ssl_DN_CN;
};

/***
 * Print all enviromental variables  
 */
void printEnviron()
{
	extern char **environ;
	
	int i = 0;
	while(environ[i]) {
	  DEBUG_PRINT_INFO("%s\n", environ[i++]);
	}
}

/***
 * Get PAM service name 
 */
const char* pamGetService()
{
	// using the "redfish" pam service config
	return "redfish";
}

/**
 * get enviroment context variables from web server 
 */
void getEnvContext(struct authContext* context)
{
	//printEnviron();

	// username and password
	context->user = getenv("REMOTE_USER");
	context->password = getenv("REMOTE_PASSWD");
	context->ssl_DN_CN = getenv("SSL_CLIENT_S_DN_CN");
}


/***
 * PAM conversation function.  
 *  
 * The PAM library uses an application-defined callback to allow
 * a direct communication between a loaded module and the 
 * application 
 */
int pamConversation(
    int nMsgs,
    const struct pam_message** messages,
    struct pam_response** resp,
    void* applicationData)
{
	struct authContext* authPtr = (struct authContext*)applicationData;
    int i;

	//DEBUG_PRINT_DEBUG("PAM authenticating nmsg:%d\n", nMsgs);

	// if no messages or messages more then max...
	if (nMsgs <= 0 || nMsgs > PAM_MAX_NUM_MSG) 
        return (PAM_CONV_ERR);
	// allocate responses
    if ((*resp = (struct pam_response*)calloc(nMsgs, sizeof **resp)) == NULL) 
        return (PAM_BUF_ERR);

    // Copy the password to the resp messages
    for (i = 0; i < nMsgs; i++)
    {
        resp[i]->resp_retcode = 0;
        resp[i]->resp = NULL;
        switch (messages[i]->msg_style) {
			case PAM_PROMPT_ECHO_OFF:
				//DEBUG_PRINT_DEBUG("PAM prompt echo off\n");
				if (strcmp(authPtr->password, "blah") == 0) {
					goto fail;
				}

				resp[i]->resp = authPtr->password ? 
					strndup(authPtr->password, PAM_MAX_MSG_SIZE) : NULL;
				if (resp[i]->resp == NULL)
					goto fail;
            break;
			case PAM_PROMPT_ECHO_ON:
				//DEBUG_PRINT_ERR("PAM prompt:%s\n", GETSTR(messages[i]->msg));
				// usually used to get the user name, but that was passed with
				// the start so this shouldn't happen, so fail
				goto fail;
            break;
			// don't do/print anything for the others, just
			// debugging here
			case PAM_ERROR_MSG:
				DEBUG_PRINT_ERR("PAM error:%s\n", GETSTR(messages[i]->msg));
            break;
			case PAM_TEXT_INFO:
				DEBUG_PRINT_ERR("PAM info:%s\n", GETSTR(messages[i]->msg));
            break;
        default:
            goto fail;
        }
    }
	//DEBUG_PRINT_DEBUG("PAM authenticating success\n");
	return (PAM_SUCCESS);

fail:
	DEBUG_PRINT_ERR("PAM authenticating fail msg:%d\n", messages[i]->msg_style);

	// free the responses and return error
	i = nMsgs;
    while (i)
        free(resp[--i]);

    free(*resp);
    *resp = NULL;
    return (PAM_CONV_ERR);
}

/**
 * Check user credentials and authorization.
 * 
 */
int pamAuth(struct authContext* context)
{
	struct pam_conv conv;
    pam_handle_t* handle = NULL;
	int status = 0, r = 0;
	int flags = PAM_SILENT; // Do not emit any messages.

	//DEBUG_PRINT_INFO("PAM authenticate %s\n", GETSTR(context->user));

    memset(&conv, 0, sizeof(conv));
	conv.appdata_ptr = (void*)context;
	conv.conv = pamConversation;

	/* The pam_start function creates the PAM context and initiates
	   the PAM transaction. It is the first of the PAM functions
	   that needs to be called by an application. */
    if ((status = pam_start(pamGetService(), context->user, &conv, &handle)) != PAM_SUCCESS) {
		DEBUG_PRINT_ERR("PAM start status:%s\n", pam_strerror(handle, status));
        return -1;
	}

	/* The pam_authenticate function is used to authenticate the user. */
	if ((status = pam_authenticate(handle, flags)) != PAM_SUCCESS)
    {
		DEBUG_PRINT_ERR("PAM authenticate status:%s\n", pam_strerror(handle, status));
        r = -1;
        goto done;
    }

	/* The pam_acct_mgmt function is used to determine if the users
	   account is valid. It checks for authentication token and account
	   expiration and verifies access restrictions. It is typically
	   called after the user has been authenticated. */

	// DISABLED - may need to be used when the roles are added
//  if ((status = pam_acct_mgmt(handle, flags)) != PAM_SUCCESS)
//  {
//  	DEBUG_PRINT_ERR("PAM acct_mgmt status:%s\n", pam_strerror(handle, status));
//      r = -1;
//      goto done;
//  }
	
	//DEBUG_PRINT_INFO("PAM authenticate complete:%s\n", pam_strerror(handle, status));
done:
	/* The pam_end function terminates the PAM transaction and is
	   the last function an application should call in the PAM context.
	   Upon return the handle pamh is no longer valid and all memory
	   associated with it will be invalid. */
    status = pam_end(handle, status);
    return r;
}

int main (int argc, char *argv[])
{
	struct authContext context;
	int authenticate = 0;

	while (FCGI_Accept() >= 0) {
		//DEBUG_PRINT_INFO("FCGI Auth req received\n");
		getEnvContext(&context);
		authenticate = pamAuth(&context);

		//DEBUG_PRINT_INFO("PAM status = %i\n", authenticate);
		/* take from apache.org documentation on mod_authnz_fcgi */
		if (authenticate != 0) {
			FCGI_printf("Status: 401\n");
			// Need to figure out why these need to be here.
			FCGI_printf("Variable-AUTHNZ_1: authnz_01\n");
			FCGI_printf("Variable-AUTHNZ_2: authnz_02\n");
			FCGI_printf("\n");
			FCGI_SetExitStatus(401);
		} else {
			FCGI_printf("Status: 200\n");
			// Need to figure out why these need to be here.
			FCGI_printf("Variable-AUTHNZ_1: authnz_01\n");
			FCGI_printf("Variable-AUTHNZ_2: authnz_02\n");
			FCGI_printf("\n");
			FCGI_SetExitStatus(200);
		}

		FCGI_Finish();
	} /* while */

	FCGI_printf("\n\n");
    return 0;
}
