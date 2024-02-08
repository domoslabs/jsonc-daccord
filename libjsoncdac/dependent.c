
#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

int _jdac_check_dependentrequired(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    json_object *j_required = json_object_object_get(jschema, "dependentRequired");
    if (!j_required)
        return JDAC_ERR_VALID;

    json_object *jdepreq_node = _jdac_output_create_and_append_node(joutput_node, "dependentRequired");

    if (!json_object_is_type(j_required, json_type_object)) {
        _jdac_output_apply_result(jdepreq_node, JDAC_ERR_SCHEMA_ERROR);
        return JDAC_ERR_SCHEMA_ERROR;
    }

    json_object_object_foreach(j_required, jprop_key, jprop_val) {
        json_object *jdepreq_key = _jdac_output_create_and_append_node(jdepreq_node, jprop_key);

        if (!json_object_is_type(jprop_val, json_type_array)) {
            _jdac_output_apply_result(jdepreq_key, JDAC_ERR_SCHEMA_ERROR);
            _jdac_output_apply_result(jdepreq_node, JDAC_ERR_SCHEMA_ERROR);
            return JDAC_ERR_SCHEMA_ERROR;
        }

        int arraylen = json_object_array_length(jprop_val);
        if (arraylen==0) {
            _jdac_output_apply_result(jdepreq_key, JDAC_ERR_SCHEMA_ERROR);
            _jdac_output_apply_result(jdepreq_node, JDAC_ERR_SCHEMA_ERROR);
            return JDAC_ERR_SCHEMA_ERROR;
        }

        for (int i = 0; i < arraylen; i++) {
            json_object *jitem = json_object_array_get_idx(jprop_val, i);
            char numstr[11];
            sprintf(numstr, "%d", i);

            if (!json_object_is_type(jitem, json_type_string)) {
                json_object *jdepreq_array_item = _jdac_output_create_and_append_node(jdepreq_key, numstr);
                _jdac_output_apply_result(jdepreq_node, JDAC_ERR_SCHEMA_ERROR);
                _jdac_output_apply_result(jdepreq_key, JDAC_ERR_SCHEMA_ERROR);
                _jdac_output_apply_result(jdepreq_array_item, JDAC_ERR_SCHEMA_ERROR);
                return JDAC_ERR_SCHEMA_ERROR;
            }
            const char *musthavestring = json_object_get_string(jitem);
            if (!json_object_object_get(jobj, musthavestring)) {
                json_object *jdepreq_array_item = _jdac_output_create_and_append_node(jdepreq_key, numstr);
                _jdac_output_apply_result(jdepreq_node, JDAC_ERR_INVALID);
                _jdac_output_apply_result(jdepreq_key, JDAC_ERR_INVALID);
                _jdac_output_apply_result(jdepreq_array_item, JDAC_ERR_INVALID);
                return JDAC_ERR_INVALID;
            }
        }
        _jdac_output_apply_result(jdepreq_key, JDAC_ERR_VALID);
    }
    _jdac_output_apply_result(jdepreq_node, JDAC_ERR_VALID);
    return JDAC_ERR_VALID;
}
