#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

int _jdac_check_pattern(json_object *jobj, json_object *jschema)
{
    json_object *jpat = json_object_object_get(jschema, "pattern");
    if(jpat) {
        if (!json_object_is_type(jpat, json_type_string))
            return JDAC_ERR_SCHEMA_ERROR;

        const char *pattern = json_object_get_string(jpat);
        const char *istr = json_object_get_string(jobj);

        if (istr) {
            int ret = _jdac_match_string_with_regex(pattern, istr);
            if (ret==JDAC_REGEX_COMPILE_FAILED)
                return JDAC_ERR_SCHEMA_ERROR;
            else if (ret==JDAC_REGEX_MATCH)
                return JDAC_ERR_VALID;
            else if (ret==JDAC_REGEX_MISMATCH)
                return JDAC_ERR_INVALID_PATTERNMATCH;
        }
    }
    return JDAC_ERR_VALID;
}
