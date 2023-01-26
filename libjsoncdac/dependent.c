
#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

int _jdac_check_dependentrequired(json_object *jobj, json_object *jschema)
{
    json_object *j_required = json_object_object_get(jschema, "dependentRequired");
    if (!j_required)
        return JDAC_ERR_VALID;

    if (!json_object_is_type(j_required, json_type_object))
        return JDAC_ERR_SCHEMA_ERROR;

    json_object_object_foreach(j_required, jprop_key, jprop_val) {
        if (!json_object_is_type(jprop_val, json_type_array))
            return JDAC_ERR_SCHEMA_ERROR;

        int arraylen = json_object_array_length(jprop_val);
        if (arraylen==0)
            return JDAC_ERR_SCHEMA_ERROR;

        for (int i = 0; i < arraylen; i++) {
            json_object *jitem = json_object_array_get_idx(jprop_val, i);
            if (!json_object_is_type(jitem, json_type_string)) 
                return JDAC_ERR_SCHEMA_ERROR;
            char *musthavestring = json_object_get_string(jitem);
            if (!json_object_object_get(jobj, musthavestring))
                return JDAC_ERR_INVALID;
        }
    }

    return JDAC_ERR_VALID;
}
