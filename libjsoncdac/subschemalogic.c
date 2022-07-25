#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

int _jdac_check_subschemalogic(json_object *jobj, json_object *jschema)
{
    int err;
    //printf("%s\n", __func__);
    json_object *jallof = json_object_object_get(jschema, "allOf");
    json_object *janyof = json_object_object_get(jschema, "anyOf");
    json_object *joneof = json_object_object_get(jschema, "oneOf");
    json_object *jnot = json_object_object_get(jschema, "not");

    // look for subschema array
    json_object *jarray = NULL;
    if (jallof) {
        jarray = jallof;
    }
    else if (janyof) {
        jarray = janyof;
    }
    else if (joneof) {
        jarray = joneof;
    }
    else if (jnot) {
        // "not" is special, and MUST be a json object
        if (json_object_is_type(jnot, json_type_object) ||
            json_object_is_type(jnot, json_type_boolean)
        ) {
            err = jdac_validate_instance(jobj, jnot);
            if (err==JDAC_ERR_VALID)
                return JDAC_ERR_INVALID_SUBSCHEMALOGIC;
            else if (err==JDAC_ERR_SCHEMA_ERROR)
                return JDAC_ERR_SCHEMA_ERROR;
            else
                return JDAC_ERR_VALID;
        }
        else
            return JDAC_ERR_SCHEMA_ERROR;
    }
    else
        return JDAC_ERR_VALID; // no subschema logic expression, continue.

    // MUST be a non-empty array
    if (!json_object_is_type(jarray, json_type_array))
        return JDAC_ERR_SCHEMA_ERROR;

    int arraylen = json_object_array_length(jarray);
    if (arraylen==0)
        return JDAC_ERR_SCHEMA_ERROR;

    int number_of_valid_schemas = 0;

    for (int i = 0; i < arraylen; i++) {
        json_object *jsubschema = json_object_array_get_idx(jarray, i);
        if (!json_object_is_type(jsubschema, json_type_object)  &&
            !json_object_is_type(jsubschema, json_type_boolean)    )
        {
            return JDAC_ERR_SCHEMA_ERROR;
        }

        err = jdac_validate_instance(jobj, jsubschema); 

        if (err==JDAC_ERR_VALID) {
            number_of_valid_schemas++;
        }
        else if (err==JDAC_ERR_SCHEMA_ERROR) {
            return JDAC_ERR_SCHEMA_ERROR;
        }
        else {
            // continue
        }
    }

    if (jallof) {
        if (number_of_valid_schemas==arraylen)
            return JDAC_ERR_VALID;
    }
    else if (janyof) {
        if (number_of_valid_schemas>0)
            return JDAC_ERR_VALID;
    }
    else if (joneof) {
        if (number_of_valid_schemas==1)
            return JDAC_ERR_VALID;
    }

    return JDAC_ERR_INVALID_SUBSCHEMALOGIC;
}
