#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

int _jdac_check_pattern(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    json_object *jpat = json_object_object_get(jschema, "pattern");

    if (!jpat)
        return JDAC_ERR_VALID;

    if (!json_object_is_type(jpat, json_type_string)) {
        json_object *jpattern_node = _jdac_output_create_and_append_node(joutput_node, "pattern");
        _jdac_output_apply_result(jpattern_node, JDAC_ERR_SCHEMA_ERROR);
        return JDAC_ERR_SCHEMA_ERROR;
    }

    const char *pattern = json_object_get_string(jpat);
    const char *istr = json_object_get_string(jobj);

    if (istr) {
        int ret = _jdac_match_string_with_regex(pattern, istr);
        if (ret==JDAC_REGEX_COMPILE_FAILED) {
            printf("jdac, failed to compile regex: %s\n", istr);
            json_object *jpattern_node = _jdac_output_create_and_append_node(joutput_node, "pattern");
            _jdac_output_apply_result(jpattern_node, JDAC_ERR_SCHEMA_ERROR);
            return JDAC_ERR_SCHEMA_ERROR;
        }
        else if (ret==JDAC_REGEX_MATCH) {
            return JDAC_ERR_VALID;
        }
        else if (ret==JDAC_REGEX_MISMATCH) {
            json_object *jpattern_node = _jdac_output_create_and_append_node(joutput_node, "pattern");
            _jdac_output_apply_result(jpattern_node, JDAC_ERR_INVALID);
            return JDAC_ERR_INVALID;
        }
    }
    return JDAC_ERR_VALID;
}
