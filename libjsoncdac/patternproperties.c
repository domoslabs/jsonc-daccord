#include <stdio.h>
#include <string.h>
#include <regex.h>        
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

int _jdac_check_patternproperties(json_object *jobj, json_object *jschema)
{
    //printf("%s\n", __func__);
    // check jobj non object is already checked

    json_object *jprops = json_object_object_get(jschema, "patternProperties");
    if (jprops) {
        if (!json_object_is_type(jprops, json_type_object))
            return JDAC_ERR_SCHEMA_ERROR;

        json_object_object_foreach(jprops, jprop_key, jprop_val) {
            //printf("key of prop is %s\n", jprop_key);
            regex_t regex;
            int reti = regcomp(&regex, jprop_key, REG_EXTENDED);
            if (reti) {
                fprintf(stderr, "Could not compile regex\n");
                return JDAC_ERR_SCHEMA_ERROR;
            }
            json_object_object_foreach(jobj, jobj_key, jobj_val) {
                //printf("  key of jobj is %s\n", jobj_key);
                reti = regexec(&regex, jobj_key, 0, NULL, 0);
                if (reti==0) {
                    //printf("match: %s\n", jobj_key);
                    int err = jdac_validate_instance(jobj_val, jprop_val);
                    if (err) {
                        regfree(&regex);
                        return err;
                    }
                }
            }
            regfree(&regex);
        }
    }
    return JDAC_ERR_VALID;    

}
