#include <syslog.h>
#include <errno.h>
#include <odata/odata.h>
#include <odata/json.h>
#include "base/str.h"

// Service Root is static taken from this file.
static const char *filename = "/etc/phit/ServiceRoot.json";

typedef struct _Provider /* Extends OL_Provider */
{
  OL_Provider base;
  /* Define provider-specific fields here */
}
Provider;

static void _Load(
  OL_Provider* self,
  OL_Scope* scope)
{
}

static void _Unload(
  OL_Provider* self,
  OL_Scope* scope)
{
  OL_Scope_DEBUG(scope, "%s(): Entered.\n", __FUNCTION__);
  free(self);
}

static void _Get(
  OL_Provider* self_,
  OL_Scope* scope,
  const OL_URI* uri)
{
  OL_Object* obj;
  size_t offset;
  OL_Result result = OL_Result_Ok;

  // Only service the root from this provider.
  if (OL_URI_Count(uri) != 0)
  {
    syslog(LOG_WARNING, "%s(): uri '%s' not supported by rootprovider.\n", 
           __FUNCTION__, OL_URI_GetName(uri, 0));
    OL_Scope_SendResult(scope, OL_Result_Failed);
    return;
  }

  char *serviceRoot = File2String(filename);
  if (!serviceRoot)
  {
    syslog(LOG_WARNING, "%s(): Error loading %s: errno=%d\n", 
           __FUNCTION__, filename, errno);
    result = OL_Result_InternalError;
    goto send_result;
  }

  if (!(obj = OL_Scope_NewObject(scope)))
  {
    result = OL_Result_InternalError;
    goto send_result;
  }

  OL_Result r = OL_Object_Deserialize(obj, serviceRoot, strlen(serviceRoot), &offset);
  if (r)
  {
    syslog(LOG_WARNING, "%s(): OL_Object_Deserialize() failed, result=%d\n", __FUNCTION__, r);
    result = OL_Result_InternalError;
    goto send_result;
  }

  OL_Scope_SendEntity(scope, obj);
  OL_Object_Release(obj);

  send_result:
    OL_Scope_SendResult(scope, result);
    free(serviceRoot);
}

static void _Post(
  OL_Provider* self,
  OL_Scope* scope,
  const OL_URI* uri,
  const OL_Object* object)
{
  OL_Scope_DEBUG(scope, "%s(): Entered.\n", __FUNCTION__);
  OL_Scope_SendResult(scope, OL_Result_NotSupported);
}

static void _Put(
  OL_Provider* self,
  OL_Scope* scope,
  const OL_URI* uri,
  const OL_Object* object)
{
  OL_Scope_DEBUG(scope, "%s(): Entered.\n", __FUNCTION__);
  OL_Scope_SendResult(scope, OL_Result_NotSupported);
}

static void _Patch(
  OL_Provider* self,
  OL_Scope* scope,
  const OL_URI* uri,
  const OL_Object* object)
{
  OL_Scope_DEBUG(scope, "%s(): Entered.\n", __FUNCTION__);
  OL_Scope_SendResult(scope, OL_Result_NotSupported);
}

static void _Delete(
  OL_Provider* self,
  OL_Scope* scope,
  const OL_URI* uri)
{
  OL_Scope_DEBUG(scope, "%s(): Entered.\n", __FUNCTION__);
  OL_Scope_SendResult(scope, OL_Result_NotSupported);
}

static OL_ProviderFT _ft =
{
  _Load,
  _Unload,
  _Get,
  _Post,
  _Put,
  _Patch,
  _Delete
};

OL_EXPORT OL_Provider* OL_ProviderEntryPoint()
{
  Provider* self;

  /* Allocate the Provider structure */
  if (!(self = (Provider*)calloc(1, sizeof(Provider))))
      return NULL;

  /* Set the function table */
  self->base.ft = &_ft;

  return &self->base;
}

