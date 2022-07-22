#include <stdio.h>
#include <string.h>
#include <regex.h>
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
            regex_t regex;
            int reti = regcomp(&regex, pattern, REG_EXTENDED);
            if (reti) {
                fprintf(stderr, "Could not compile regex\n");
                return JDAC_ERR_SCHEMA_ERROR;
            }
           reti = regexec(&regex, istr, 0, NULL, 0);
           regfree(&regex);
           if (reti==0) {
                return JDAC_ERR_VALID;
           }
           return JDAC_ERR_INVALID_PATTERNMATCH;
        }
    }
    return JDAC_ERR_VALID;
}
