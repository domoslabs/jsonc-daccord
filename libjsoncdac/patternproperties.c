#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

int _jdac_check_patternproperties(json_object *jobj, json_object *jschema)
{
    //printf("%s\n", __func__);
    // check jobj non object is already checked

    json_object *jpatprops = json_object_object_get(jschema, "patternProperties");
    if (jpatprops) {
        if (!json_object_is_type(jpatprops, json_type_object))
            return JDAC_ERR_SCHEMA_ERROR;

        json_object_object_foreach(jpatprops, jprop_key, jprop_val) {
            json_object_object_foreach(jobj, jobj_key, jobj_val) {
                int ret = _jdac_match_string_with_regex(jprop_key, jobj_key);
                if (ret==JDAC_REGEX_COMPILE_FAILED)
                    return JDAC_ERR_SCHEMA_ERROR;
                else if (ret==JDAC_REGEX_MATCH) {
                    int err = _jdac_validate_instance(jobj_val, jprop_val);
                    if (err)
                        return err;
                }
                else if (ret==JDAC_REGEX_MISMATCH)
                {
                    //
                }
            }
        }
    }
    return JDAC_ERR_VALID;    
}
