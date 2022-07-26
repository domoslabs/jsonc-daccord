#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

int _jdac_check_additionalproperties(json_object *jobj, json_object *jschema)
{
    json_object *jaddprops = json_object_object_get(jschema, "additionalProperties");

    if (!jaddprops)
        return JDAC_ERR_VALID;

    if (!json_object_is_type(jaddprops, json_type_object) &&
        !json_object_is_type(jaddprops, json_type_boolean))
        return JDAC_ERR_SCHEMA_ERROR;

    json_object *jprops = json_object_object_get(jschema, "properties");
#ifdef JDAC_PATTERNPROPERTIES
    json_object *jpatprops = json_object_object_get(jschema, "patternProperties");
#endif

    json_object_object_foreach(jobj, jobj_key, jobj_val) {

        // if an instance key is found in properties, it is not an additional property
        if (jprops) {
            json_object *jprop = json_object_object_get(jprops, jobj_key);
            if (jprop)
                continue; // ignore properties
        }

        // if an instance key has a match in patternProperties, it is not an additional property
#ifdef JDAC_PATTERNPROPERTIES
        int foundpatternproperty=0;
        if (jpatprops) {
            if (json_object_is_type(jpatprops, json_type_object)) {
                json_object_object_foreach(jpatprops, jpat_key, jpat_val) {
                    int ret = _jdac_match_string_with_regex(jpat_key, jobj_key);
                    if (ret==JDAC_REGEX_MATCH) {
                        foundpatternproperty = 1;
                        break;
                    }
                }
            }
            if (foundpatternproperty)
                continue;
        }
#endif

        // by this point we consider the instance to be an additional property
        int err = _jdac_validate_instance(jobj_val, jaddprops);
        if (err)
            return err;

    }


    return JDAC_ERR_VALID;
}
