#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

int _jdac_check_prefixItems(json_object *jobj, json_object *jschema)
{
    json_object *jprefix = json_object_object_get(jschema, "prefixItems");

    if (!jprefix)
        return JDAC_ERR_VALID;

    if (!json_object_is_type(jprefix, json_type_array))
        return JDAC_ERR_SCHEMA_ERROR;

    int jobj_arraylen = json_object_array_length(jobj);
    int jprefix_arraylen = json_object_array_length(jprefix);

    int items_to_check = jobj_arraylen < jprefix_arraylen ? jobj_arraylen : jprefix_arraylen;

    for (int i=0; i<items_to_check; i++) {
        json_object *iobj = json_object_array_get_idx(jobj, i);
        json_object *ischema = json_object_array_get_idx(jprefix, i);
        int err = jdac_validate_instance(iobj, ischema);
        if (err)
            return JDAC_ERR_INVALID_PREFIXITEMS;
    }
    return JDAC_ERR_VALID;
}
