#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

int _jdac_check_ref(json_object *jobj, json_object *jschema, storage_node *storage_list)
{
// #ifdef JDAC_STORE
    json_object *jref = json_object_object_get(jschema, "$ref");
    if (jref) {
        const char *refstr = json_object_get_string(jref);
        if (refstr) {
            json_object *jschema_from_resolved_uri = _jdac_store_resolve(storage_list, refstr);
            if (!jschema_from_resolved_uri)
                return JDAC_ERR_INVALID_REF;
            int err = _jdac_validate_instance(jobj, jschema_from_resolved_uri);
            return err;
        }
    }
// #endif
    return JDAC_ERR_VALID;
}