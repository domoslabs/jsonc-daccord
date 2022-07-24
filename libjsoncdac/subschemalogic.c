#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

enum subschematype {
    JDAC_ALLOF = 0,
    JDAC_ANYOF,
    JDAC_ONEOF
};

// shall return valid or invalid based on subschema type
int _jdac_test_subschema_array(json_object *jobj, json_object *jsubschema_array, enum subschematype type)
{
    if (jsubschema_array==NULL)
        return JDAC_ERR_VALID;

    // MUST be a non-empty array
    if (!json_object_is_type(jsubschema_array, json_type_array))
        return JDAC_ERR_SCHEMA_ERROR;

    int arraylen = json_object_array_length(jsubschema_array);
    if (arraylen==0)
        return JDAC_ERR_SCHEMA_ERROR;

    int number_of_valid_schemas = 0;

    for (int i = 0; i < arraylen; i++) {
        json_object *jsubschema = json_object_array_get_idx(jsubschema_array, i);
        if (!json_object_is_type(jsubschema, json_type_object)  &&
            !json_object_is_type(jsubschema, json_type_boolean)    )
        {
            return JDAC_ERR_SCHEMA_ERROR;
        }

        int err = jdac_validate_instance(jobj, jsubschema); 

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

    if (type==JDAC_ALLOF) {
        if (number_of_valid_schemas==arraylen)
            return JDAC_ERR_VALID;
    }
    else if (type==JDAC_ANYOF) {
        if (number_of_valid_schemas>0)
            return JDAC_ERR_VALID;
    }
    else if (type==JDAC_ONEOF) {
        if (number_of_valid_schemas==1)
            return JDAC_ERR_VALID;
    }
    return JDAC_ERR_INVALID_SUBSCHEMALOGIC;
}

int _jdac_check_subschemalogic(json_object *jobj, json_object *jschema)
{
    int err;
    json_object *jarray;

    jarray = json_object_object_get(jschema, "allOf");
    err = _jdac_test_subschema_array(jobj, jarray, JDAC_ALLOF);
    if (err)
        return JDAC_ERR_INVALID_SUBSCHEMALOGIC;

    jarray = json_object_object_get(jschema, "anyOf");
    err = _jdac_test_subschema_array(jobj, jarray, JDAC_ANYOF);
    if (err)
        return JDAC_ERR_INVALID_SUBSCHEMALOGIC;

    jarray = json_object_object_get(jschema, "oneOf");
    err = _jdac_test_subschema_array(jobj, jarray, JDAC_ONEOF);
    if (err)
        return JDAC_ERR_INVALID_SUBSCHEMALOGIC;

    json_object *jnot = json_object_object_get(jschema, "not");
    if (jnot) {
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

    return JDAC_ERR_VALID;
}
