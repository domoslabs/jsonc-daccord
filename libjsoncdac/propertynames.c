#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

int _jdac_check_propertynames(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    json_object *jpropnames_schema = json_object_object_get(jschema, "propertyNames");
    if (!jpropnames_schema)
        return JDAC_ERR_VALID;

    json_object *jpropnames_node = _jdac_output_create_and_append_node(joutput_node, "propertyNames");

    if (!json_object_is_type(jpropnames_schema, json_type_object) &&
        !json_object_is_type(jpropnames_schema, json_type_boolean)
    ) {
        _jdac_output_apply_result(jpropnames_node, JDAC_ERR_SCHEMA_ERROR);
        return JDAC_ERR_SCHEMA_ERROR;
    }

    int propnames_ok = 1;
    json_object_object_foreach(jobj, jprop_key, jprop_val) {
        if (jprop_val) {}
        json_object *jkey_node = _jdac_output_create_and_append_node(jpropnames_node, jprop_key);
        json_object *jprop = json_object_new_string(jprop_key);
        int err = _jdac_validate_instance(jprop, jpropnames_schema, jkey_node);
        _jdac_output_apply_result(jkey_node, err);
        json_object_put(jprop);
        if (err!=JDAC_ERR_VALID) {
            propnames_ok=0;
        }
    }
    int ret = propnames_ok==1 ? JDAC_ERR_VALID:JDAC_ERR_INVALID;
    _jdac_output_apply_result(jpropnames_node, ret);
    return ret;    
}
