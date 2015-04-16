#include <odata/odata.h>
#include <syslog.h>

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
  syslog(LOG_INFO, "%s(): Entered.\n", __FUNCTION__);
  //OL_Scope_DEBUG(scope, "%s(): Entered.\n", __FUNCTION__);
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

  OL_Scope_SendBeginEntitySet(scope);

  if (!(obj = OL_Scope_NewObject(scope)))
      return ;

  OL_Object_AddInt64(obj, "Id", 1001);
  OL_Object_AddString(obj, "Color", "Blue");
  OL_Object_AddString(obj, "Model", "Polara 500");
  OL_Object_AddInt32(obj, "Weight", 4200);

  OL_Scope_SendEntity(scope, obj);
  OL_Object_Release(obj);

  if (!(obj = OL_Scope_NewObject(scope)))
      return ;

  OL_Object_AddInt64(obj, "Id", 1002);
  OL_Object_AddString(obj, "Color", "Gold");
  OL_Object_AddString(obj, "Model", "Imperial Crown");
  OL_Object_AddInt32(obj, "Weight", 5100);

  OL_Scope_SendEntity(scope, obj);
  OL_Object_Release(obj);

  OL_Scope_SendEndEntitySet(scope);
  OL_Scope_SendResult(scope, OL_Result_Ok);
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

