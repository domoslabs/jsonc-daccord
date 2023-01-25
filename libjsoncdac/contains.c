#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"


int _jdac_check_contains_and_minmaxcontains(json_object *jobj, json_object *jschema)
{
    int err;

    json_object *jcontains = json_object_object_get(jschema, "contains");
    if (!jcontains)
        return JDAC_ERR_VALID;

    if (!json_object_is_type(jcontains, json_type_object) &&
        !json_object_is_type(jcontains, json_type_boolean))
        return JDAC_ERR_SCHEMA_ERROR;

    int arraylen = json_object_array_length(jobj);

    int match_count=0;
    for(int i=0; i<arraylen; i++)
    {
        json_object *iobj = json_object_array_get_idx(jobj, i);
        err = _jdac_validate_instance(iobj, jcontains);
        if (err==JDAC_ERR_VALID)
            match_count++;
        else if (err==JDAC_ERR_SCHEMA_ERROR)
            return JDAC_ERR_SCHEMA_ERROR;
    }

    json_object *jmaxcontains = json_object_object_get(jschema, "maxContains");
    if (jmaxcontains) {
        int max = json_object_get_int(jmaxcontains);
        if (match_count>max)
            return JDAC_ERR_INVALID_CONTAINS;
    }

    json_object *jmincontains = json_object_object_get(jschema, "minContains");
    if (jmincontains) {
        int min = json_object_get_int(jmincontains);
        if (min==0)
            return JDAC_ERR_VALID;
        else if (match_count<min)
            return JDAC_ERR_INVALID_CONTAINS;
    }

    if (match_count==0)
        return JDAC_ERR_INVALID_CONTAINS;

    return JDAC_ERR_VALID;
}
