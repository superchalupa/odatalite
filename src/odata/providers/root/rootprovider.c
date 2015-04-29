#include <errno.h>
#include <odata/odata.h>
#include <odata/json.h>
#include "base/str.h"

// Service Root is static taken from this file.
static char *serviceRootFile = "/etc/phit/ServiceRoot.json";
static char *odataServiceFile = "/etc/phit/OdataService.json";
static char *metadataFile = "/etc/phit/ServiceRoot.metadata";

typedef struct _Provider /* Extends OL_Provider */
{
  OL_Provider base;
  /* Define provider-specific fields here */
}
Provider;

static void _Unload(
  OL_Provider* self,
  OL_Scope* scope)
{
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
  size_t count = OL_URI_Count(uri);

  char *filename = serviceRootFile;

  if (count == 1)
  {
    const char *endpoint = OL_URI_GetName(uri, 0);
    if (!strcmp(endpoint, "odata"))
    {
      filename = odataServiceFile;
    }
    else if (!strcmp(endpoint, "$metadata"))
    {
      filename = metadataFile;
    }
    else
    {
      OL_Scope_ERR(scope, "uri '%s' not supported by rootprovider.\n",
                          endpoint);
      OL_Scope_SendResult(scope, OL_Result_Failed);
      return;
    }
  }

  char *data = File2String(filename);
  if (!data)
  {
    OL_Scope_ERR(scope, "Error loading %s: errno=%d\n", filename, errno);
    result = OL_Result_InternalError;
    goto send_result;
  }

  if (!(obj = OL_Scope_NewObject(scope)))
  {
    result = OL_Result_InternalError;
    goto send_result;
  }

  if (strstr(data,"<?xml"))
  {
    OL_Scope_SendMetadataXML(scope, data, strlen(data));
    OL_Scope_SendResult(scope, OL_Result_Ok);
    OL_Object_Release(obj);
    free(data);
    return;
  }

  /* TODO: These belong in the general odata handler.
           Needs the following info: 
           ServiceRoot: /rest/v1
           SchemaVersion: 0.94.0
   */
  if (!count)
  {
    OL_Object_AddString(obj, "@odata.id", "/rest/v1");
    OL_Object_AddString(obj, "@odata.type", "#ServiceRoot.0.94.0.ServiceRoot");
    OL_Object_AddString(obj, "Modified", "2013-01-31T23:45:04+00:00");
    OL_Object_AddString(obj, "RedfishVersion", "0.94.0");
    OL_Object_AddString(obj, "UUID", "00000000-0000-0000-0000-000000000000");

    // These go in the provider itself.
    OL_Object_AddString(obj, "Id", "RootService");
    OL_Object_AddString(obj, "Name", "Root Service");
  }

  OL_Result r = OL_Object_Deserialize(obj, data, strlen(data), &offset);
  if (r)
  {
    OL_Scope_ERR(scope, "OL_Object_Deserialize() failed, result=%d\n", r);
    result = OL_Result_InternalError;
    goto send_result;
  }

  OL_Scope_SendEntity(scope, obj);
  OL_Object_Release(obj);

  send_result:
    OL_Scope_SendResult(scope, result);
    free(data);
}

static void _Post(
  OL_Provider* self,
  OL_Scope* scope,
  const OL_URI* uri,
  const OL_Object* object)
{
  OL_Scope_SendResult(scope, OL_Result_NotSupported);
}

static void _Put(
  OL_Provider* self,
  OL_Scope* scope,
  const OL_URI* uri,
  const OL_Object* object)
{
  OL_Scope_SendResult(scope, OL_Result_NotSupported);
}

static void _Patch(
  OL_Provider* self,
  OL_Scope* scope,
  const OL_URI* uri,
  const OL_Object* object)
{
  OL_Scope_SendResult(scope, OL_Result_NotSupported);
}

static void _Delete(
  OL_Provider* self,
  OL_Scope* scope,
  const OL_URI* uri)
{
  OL_Scope_SendResult(scope, OL_Result_NotSupported);
}

static OL_ProviderFT _ft =
{
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

