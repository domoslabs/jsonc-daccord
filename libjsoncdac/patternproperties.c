#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

int _jdac_check_patternproperties(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    //printf("%s\n", __func__);
    // check jobj non object is already checked

    json_object *jpatprops = json_object_object_get(jschema, "patternProperties");
    if (!jpatprops) {
        return JDAC_ERR_VALID;
    }

    json_object *jpatprop_node = _jdac_output_create_and_append_node(joutput_node, "patternProperties");

    if (!json_object_is_type(jpatprops, json_type_object)) {
        _jdac_output_apply_result(jpatprop_node, JDAC_ERR_SCHEMA_ERROR);
        return JDAC_ERR_SCHEMA_ERROR;
    }

    int patternproperties_ok = 1;
    json_object_object_foreach(jpatprops, jprop_key, jprop_val) {
        json_object_object_foreach(jobj, jobj_key, jobj_val) {
            int ret = _jdac_match_string_with_regex(jprop_key, jobj_key);
            if (ret==JDAC_REGEX_COMPILE_FAILED) {
                _jdac_output_apply_result(jpatprop_node, JDAC_ERR_SCHEMA_ERROR);
                return JDAC_ERR_SCHEMA_ERROR;
            }
            else if (ret==JDAC_REGEX_MATCH) {
                json_object* jpat_item = _jdac_output_create_and_append_node_concatnames(jpatprop_node, jprop_key, jobj_key);
                int err = _jdac_validate_instance(jobj_val, jprop_val, jpat_item);
                _jdac_output_apply_result(jpat_item, err);
                if (err!=JDAC_ERR_VALID) {
                    patternproperties_ok = 0;
                }
            }
            else if (ret==JDAC_REGEX_MISMATCH)
            {
                //
            }
        }
    }
    int ret = patternproperties_ok==1 ? JDAC_ERR_VALID : JDAC_ERR_INVALID;
    _jdac_output_apply_result(jpatprop_node, ret);
    return ret;    
}
