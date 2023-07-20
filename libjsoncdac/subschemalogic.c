#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

enum subschematype {
    JDAC_ALLOF = 0,
    JDAC_ANYOF,
    JDAC_ONEOF
};


// shall return valid or invalid based on subschema type
int _jdac_test_subschema_array(json_object *jobj, json_object *jsubschema_array, enum subschematype type, json_object *joutput_node)
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

        json_object *some_node = _jdac_output_create_node("some");
        int err = _jdac_validate_instance(jobj, jsubschema, some_node); 
        json_object_put(some_node); // we don't need it anyway

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
    return JDAC_ERR_INVALID;
}

int _jdac_check_subschemalogic(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    int err;
    json_object *jarray;


    int allOf_ok = 1;
    jarray = json_object_object_get(jschema, "allOf");
    if (jarray) {
        json_object *jallOf_node = _jdac_output_create_node("allOf");
        err = _jdac_test_subschema_array(jobj, jarray, JDAC_ALLOF, jallOf_node);
        if (err) {
            allOf_ok=0;
            _jdac_output_apply_result(jallOf_node, err);
            _jdac_output_append_node(joutput_node, jallOf_node);
            if (err==JDAC_ERR_SCHEMA_ERROR) {
                return err;
            }
        } else {
            json_object_put(jallOf_node);
        }
    }

    int anyOf_ok = 1;
    jarray = json_object_object_get(jschema, "anyOf");
    if (jarray) {
        json_object *janyOf_node = _jdac_output_create_node("anyOf");
        err = _jdac_test_subschema_array(jobj, jarray, JDAC_ANYOF, janyOf_node);
        if (err) {
            anyOf_ok=0;
            _jdac_output_apply_result(janyOf_node, err);
            _jdac_output_append_node(joutput_node, janyOf_node);
            if (err==JDAC_ERR_SCHEMA_ERROR) {
                return err;
            }
        } else {
            json_object_put(janyOf_node);
        }
    }

    int oneOf_ok = 1;
    jarray = json_object_object_get(jschema, "oneOf");
    if (jarray) {
        json_object *joneOf_node = _jdac_output_create_node("oneOf");
        err = _jdac_test_subschema_array(jobj, jarray, JDAC_ONEOF, joneOf_node);
        if (err) {
            oneOf_ok=0;
            _jdac_output_apply_result(joneOf_node, err);
            _jdac_output_append_node(joutput_node, joneOf_node);
            if (err==JDAC_ERR_SCHEMA_ERROR) {
                return err;
            }
        } else {
            json_object_put(joneOf_node);
        }
    }

    int not_ok = 1;
    json_object *jnot = json_object_object_get(jschema, "not");
    if (jnot) {
        json_object *jnot_node = _jdac_output_create_and_append_node(joutput_node, "not");

        // "not" is special, and MUST be a json object
        if (json_object_is_type(jnot, json_type_object) ||
            json_object_is_type(jnot, json_type_boolean)
        ) {
            err = _jdac_validate_instance(jobj, jnot, jnot_node);
            if (err==JDAC_ERR_VALID) {
                _jdac_output_apply_result(jnot_node, JDAC_ERR_INVALID);
                not_ok=0;
            }
            else if (err==JDAC_ERR_INVALID) {
                _jdac_output_apply_result(jnot_node, JDAC_ERR_VALID);
            }
            else if (err==JDAC_ERR_SCHEMA_ERROR) {
                _jdac_output_apply_result(jnot_node, JDAC_ERR_SCHEMA_ERROR);
                return JDAC_ERR_SCHEMA_ERROR;
            }
            else {
                printf("%s: unexpected return from not operation (%d)\n", __func__, err);
                _jdac_output_apply_result(jnot_node, JDAC_ERR_SCHEMA_ERROR);
                return JDAC_ERR_SCHEMA_ERROR;
            }
        }
        else {
            _jdac_output_apply_result(jnot_node, JDAC_ERR_SCHEMA_ERROR);
            return JDAC_ERR_SCHEMA_ERROR;
        }
    }

    int if_ok = 1;
    json_object *if_schema = json_object_object_get(jschema, "if");
    json_object *then_schema = json_object_object_get(jschema, "then");
    json_object *else_schema = json_object_object_get(jschema, "else");
    if (if_schema) {
        json_object *jif_node = _jdac_output_create_and_append_node(joutput_node, "if");
        err = _jdac_validate_instance(jobj, if_schema, jif_node);
        _jdac_output_apply_result(jif_node, err);

        if (err==JDAC_ERR_SCHEMA_ERROR) {
            return err;
        }

        if (err==JDAC_ERR_VALID && then_schema) {
            json_object *jthen_node = _jdac_output_create_and_append_node(joutput_node, "then");
            err = _jdac_validate_instance(jobj, then_schema, jthen_node);
            _jdac_output_apply_result(jthen_node, err);
            if (err) {
                if_ok = 0;
                if (err==JDAC_ERR_SCHEMA_ERROR) {
                    return err;
                }
            }
        }
        else if (err==JDAC_ERR_INVALID && else_schema ) {
            json_object *jelse_node = _jdac_output_create_and_append_node(joutput_node, "jnode_else");
            err = _jdac_validate_instance(jobj, else_schema, jelse_node);
            _jdac_output_apply_result(jelse_node, err);
            if (err) {
                if_ok = 0;
                if (err==JDAC_ERR_SCHEMA_ERROR) {
                    return err;
                }
            }
        }
    }

    int subschemalogic_ret = allOf_ok==1 && anyOf_ok==1 && oneOf_ok==1 && not_ok==1 && if_ok==1 ? JDAC_ERR_VALID:JDAC_ERR_INVALID;
    return subschemalogic_ret;
}

