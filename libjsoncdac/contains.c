#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"


int _jdac_check_contains_and_minmaxcontains(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    int err;

    json_object *jcontains = json_object_object_get(jschema, "contains");
    if (!jcontains)
        return JDAC_ERR_VALID;

    json_object *jcontains_node = _jdac_output_create_and_append_node(joutput_node, "contains");

    if (!json_object_is_type(jcontains, json_type_object) &&
        !json_object_is_type(jcontains, json_type_boolean))
    {
        _jdac_output_apply_result(jcontains_node, JDAC_ERR_SCHEMA_ERROR);
        return JDAC_ERR_SCHEMA_ERROR;
    }

    int arraylen = json_object_array_length(jobj);

    int match_count=0;
    for(int i=0; i<arraylen; i++)
    {
        json_object *iobj = json_object_array_get_idx(jobj, i);
        char numstr[11];
        sprintf(numstr, "%d", i);
        json_object *iobj_node_tmp = _jdac_output_create_node(numstr);

        err = _jdac_validate_instance(iobj, jcontains, iobj_node_tmp);
        if (err==JDAC_ERR_VALID) {
            match_count++;
        }
        else if (err==JDAC_ERR_SCHEMA_ERROR) {
            _jdac_output_apply_result(iobj_node_tmp, JDAC_ERR_SCHEMA_ERROR);
            _jdac_output_append_node(jcontains_node, iobj_node_tmp);
            _jdac_output_apply_result(jcontains_node, JDAC_ERR_SCHEMA_ERROR);
            return JDAC_ERR_SCHEMA_ERROR;
        } else {
            printf("%s: unhandeled error (%d)\n", __func__, err);
        }
        json_object_put(iobj_node_tmp);
    }

    int maxcontains_ok = 1;
    json_object *jmaxcontains = json_object_object_get(jschema, "maxContains");
    if (jmaxcontains) {
        int max = json_object_get_int(jmaxcontains);
        if (match_count>max) {
            json_object *jmaxcont_node = _jdac_output_create_and_append_node(jcontains_node, "maxContains");
            _jdac_output_apply_result(jmaxcont_node, JDAC_ERR_INVALID);
            maxcontains_ok=0;
        }
    }

    int mincontains_ok = 1;
    json_object *jmincontains = json_object_object_get(jschema, "minContains");
    if (jmincontains) {
        int min = json_object_get_int(jmincontains);
        if (match_count<min && min!=0) {
            json_object *jmincont_node = _jdac_output_create_and_append_node(jcontains_node, "minContains");
            _jdac_output_apply_result(jmincont_node, JDAC_ERR_INVALID);
            mincontains_ok=0;
        }
    }

    int ret = maxcontains_ok==1 && mincontains_ok==1 ? JDAC_ERR_VALID:JDAC_ERR_INVALID;
    _jdac_output_apply_result(jcontains_node, ret);
    return ret;
}
