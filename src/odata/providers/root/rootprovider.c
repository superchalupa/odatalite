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
  OL_Scope_DEBUG(scope, "%s(): Entered.\n", __FUNCTION__);

  OL_Object* obj;
  size_t offset;

  char *serviceRoot = File2String(filename);
  if (!serviceRoot)
  {
    syslog(LOG_WARNING, "%s(): Error loading %s: errno=%d\n", 
           __FUNCTION__, filename, errno);
    OL_Scope_SendResult(scope, OL_Result_InternalError);
  }

  if (!(obj = OL_Scope_NewObject(scope)))
      return ;

  OL_Result r = OL_Object_Deserialize(obj, serviceRoot, strlen(serviceRoot), &offset);
  if (r)
  {
    syslog(LOG_WARNING, "%s(): OL_Object_Deserialize() failed, result=%d\n", __FUNCTION__, r);
    OL_Scope_SendResult(scope, OL_Result_InternalError);
  }

  OL_Scope_SendEntity(scope, obj);
  OL_Object_Release(obj);

  OL_Scope_SendResult(scope, OL_Result_Ok);
  free(serviceRoot);

  return;
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

