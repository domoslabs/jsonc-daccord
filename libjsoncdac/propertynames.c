#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

int _jdac_check_propertynames(json_object *jobj, json_object *jschema)
{
    json_object *jpropnames_schema = json_object_object_get(jschema, "propertyNames");
    if (!jpropnames_schema)
        return JDAC_ERR_VALID;

    if (!json_object_is_type(jpropnames_schema, json_type_object) &&
        !json_object_is_type(jpropnames_schema, json_type_boolean)
    )
        return JDAC_ERR_SCHEMA_ERROR;

    json_object_object_foreach(jobj, jprop_key, jprop_val) {
        json_object *jprop = json_object_new_string(jprop_key);
        int err = jdac_validate_instance(jprop, jpropnames_schema);
        json_object_put(jprop);
        if (err)
            return err;
    }

    return JDAC_ERR_VALID;    
}
