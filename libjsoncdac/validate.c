#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <json-c/json.h>

#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

json_object *json = NULL;
json_object *schema = NULL;
json_object *defs = NULL;


#ifdef JDAC_STORE
    static storage_node *storagelist_head = NULL;
#endif

static char* jdacerrstr[JDAC_ERR_MAX] = {
    "VALID",
    "GENERAL ERROR",
    "JSON FILE NOT FOUND",
    "SCHEMA FILE NOT FOUND",
    "WRONG ARGUEMNTS GIVEN",
    "SCHEMA ERROR",
    "INVALID",
    "INVALID TYPE",
    "INVALID REQUIRED",
    "INVALID SUBSCHEMA LOGIC (allOf, anyOf, oneOf, or not)",
    "INVALID CONST",
    "INVALID ENUMS",
    "INVALID STRING LENGTH",
    "INVALID UNIQUE ITEMS",
    "INVALID UNIQUE CONTAINS",
    "INVALID PREFIXITEMS",
    "INVALID ITEMS",
    "INVALID ARRAY LENGTH",
    "INVALID NUMBER",
    "INVALID REFERENCE",
    "PATTERN NO MATCH",
    "REGEX MISMATCH",
    "REGEX MATCH",
    "REGEX COMPILE FAILED"
};

const char* jdac_errorstr(unsigned int jdac_errors)
{
    if (jdac_errors<JDAC_ERR_MAX) {
        return jdacerrstr[jdac_errors];
    }
    return NULL;
}

int _jdac_load(const char *jsonfile, const char *jsonschema)
{
    json = json_object_from_file(jsonfile);
    if (json==NULL) {
        return JDAC_ERR_JSON_NOT_FOUND;
    }

    schema = json_object_from_file(jsonschema);
    if (schema==NULL) {
        json_object_put(json);
        return JDAC_ERR_SCHEMA_NOT_FOUND;
    }

    return JDAC_ERR_VALID;
}

int __jdac_inspect_type(json_object *jobj, const char *type, json_object *joutput_node)
{
    if (strcmp(type,"object")==0) {
        if (json_object_is_type(jobj, json_type_object))
            return JDAC_ERR_VALID;
    }
    else if (strcmp(type,"array")==0) {
        if (json_object_is_type(jobj, json_type_array))
            return JDAC_ERR_VALID;
    }
    else if (strcmp(type,"string")==0) {
        if (json_object_is_type(jobj, json_type_string))
            return JDAC_ERR_VALID;
    }
    else if (strcmp(type,"integer")==0) {
        if (json_object_is_type(jobj, json_type_int))
            return JDAC_ERR_VALID;
        if (json_object_is_type(jobj, json_type_double)) {
            double value = json_object_get_double(jobj);
            if (value==round(value)) // "zero fractional part is an integer"
                return JDAC_ERR_VALID;
        }
    }
    else if (strcmp(type,"double")==0) {
        if (json_object_is_type(jobj, json_type_double))
            return JDAC_ERR_VALID;
    }
    else if (strcmp(type,"number")==0) {
        if (json_object_is_type(jobj, json_type_double) ||
            json_object_is_type(jobj, json_type_int))
            return JDAC_ERR_VALID;
    }
    else if (strcmp(type,"boolean")==0) {
        if (json_object_is_type(jobj, json_type_boolean))
            return JDAC_ERR_VALID;
    }
    else if (strcmp(type,"null")==0) {
        if (json_object_is_type(jobj, json_type_null))
            return JDAC_ERR_VALID;
    }
    else {
        printf("WARN unknown type in check type %s\n", type);
        json_object *jnode = _jdac_output_create_and_append_node(joutput_node, "type");
        _jdac_output_apply_result(jnode, JDAC_ERR_SCHEMA_ERROR);
        return JDAC_ERR_SCHEMA_ERROR;
    }
    json_object *jnode = _jdac_output_create_and_append_node(joutput_node, "type");
    _jdac_output_apply_result(jnode, JDAC_ERR_INVALID);
 
    return JDAC_ERR_INVALID;
}


int _jdac_check_bool(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    // check if jschema is a bool, true or false
    int err;
    if (json_object_is_type(jschema, json_type_boolean)) {
        json_object *jnode = _jdac_output_create_and_append_node(joutput_node, "bool");
        json_bool value = json_object_get_boolean(jschema);
        err = value==0 ? JDAC_ERR_INVALID : JDAC_ERR_VALID;
        _jdac_output_apply_result(jnode, err);
        return err;
    }
    return JDAC_ERR_VALID;
}

int _jdac_check_type(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    json_object *jtype = json_object_object_get(jschema, "type");
    if  (jtype==NULL) {
        return JDAC_ERR_VALID;
    }
    else if (json_object_is_type(jtype, json_type_string)) {
        const char *type = json_object_get_string(jtype);
        return __jdac_inspect_type(jobj, type, joutput_node);
    }
    else if (json_object_is_type(jtype, json_type_array)) {
        int arraylen = json_object_array_length(jtype);
        for (int i=0; i<arraylen; i++) {
            json_object *iobj = json_object_array_get_idx(jtype, i);
            if (!json_object_is_type(iobj, json_type_string))
                goto check_type_schema_error;
            const char *type = json_object_get_string(iobj);
            int err = __jdac_inspect_type(jobj, type, joutput_node);
            if (err==JDAC_ERR_VALID)
                return JDAC_ERR_VALID;
        }
        json_object *jnode = _jdac_output_create_and_append_node(joutput_node, "type");
        _jdac_output_apply_result(jnode, JDAC_ERR_INVALID);
        return JDAC_ERR_INVALID;
    }
check_type_schema_error:
    json_object *jnode = _jdac_output_create_and_append_node(joutput_node, "type");
    _jdac_output_apply_result(jnode, JDAC_ERR_SCHEMA_ERROR);
    return JDAC_ERR_SCHEMA_ERROR;
}

int _jdac_check_required(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    //printf("%s\n%s\n", __func__, json_object_to_json_string(jobj));
    json_object *jarray = json_object_object_get(jschema, "required");
    if (!jarray) {
        return JDAC_ERR_VALID;
    }

    json_object *jrequired_node = _jdac_output_create_and_append_node(joutput_node, "required");
    int missing_required_key = 0;

    int arraylen = json_object_array_length(jarray);
    for (int i = 0; i < arraylen; i++) {
        json_object *iobj = json_object_array_get_idx(jarray, i);
        const char *key = json_object_get_string(iobj);
        if (key) {
            //printf("%s\n", key);
            // use json_object_object_get_ex becuase of json_type_null types
            json_object *required_object = NULL;
            int err = json_object_object_get_ex(jobj, key, &required_object);
            if (err==0) {
                // printf("required key missing: %s\n", key);
                json_object *jkeynode = _jdac_output_create_and_append_node(jrequired_node, key);
                _jdac_output_apply_result(jkeynode, JDAC_ERR_INVALID);
                missing_required_key=1;
            }
        }
    }
    int ret = missing_required_key==1 ? JDAC_ERR_INVALID:JDAC_ERR_VALID;
    _jdac_output_apply_result(jrequired_node, ret);
    return ret;
}

int _jdac_check_properties(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    // printf("%s\n", __func__);

    json_object *jprops = json_object_object_get(jschema, "properties");
    if (!jprops) {
        return JDAC_ERR_VALID;
    }

    json_object *jproperties_node = _jdac_output_create_and_append_node(joutput_node, "properties");
    int properties_valid = 1;
    json_object_object_foreach(jprops, jprop_key, jprop_val) {
        // printf("key of prop is %s\n", jprop_key);
        json_object *iobj = json_object_object_get(jobj, jprop_key);
        //printf("iobj %s type %d\nkey %s\nval %s\n", json_object_get_string(iobj), json_object_get_type(iobj), jprop_key, json_object_get_string(jprop_val));
        if (iobj) {
            json_object *jprop_item_tmp_node = _jdac_output_create_node(jprop_key);
            int err = _jdac_validate_instance(iobj, jprop_val, jprop_item_tmp_node);
            if (err!=JDAC_ERR_VALID) {
                properties_valid=0;
                _jdac_output_apply_result(jprop_item_tmp_node, err);
                _jdac_output_append_node(jproperties_node, jprop_item_tmp_node);
            } else {
                json_object_put(jprop_item_tmp_node);
            }
        }
    }
    int ret = properties_valid==1 ? JDAC_ERR_VALID : JDAC_ERR_INVALID;
    _jdac_output_apply_result(jproperties_node, ret);
    return ret;
}

int _jdac_check_prefixItems_and_items(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    json_object *jprefixitems = json_object_object_get(jschema, "prefixItems");
    json_object *jitems = json_object_object_get(jschema, "items");

    int prefixitems_ok = 1;
    int items_ok = 1;

    if (jprefixitems) {
        json_object *jprefixitems_node = _jdac_output_create_and_append_node(joutput_node, "prefixItems");

        if (!json_object_is_type(jprefixitems, json_type_array)) {
            _jdac_output_apply_result(jprefixitems_node, JDAC_ERR_SCHEMA_ERROR);
            return JDAC_ERR_SCHEMA_ERROR;
        }

        int jobj_arraylen = json_object_array_length(jobj);
        int prefixitems_arraylen = json_object_array_length(jprefixitems);
        for (int i=0; i<jobj_arraylen && i<prefixitems_arraylen; i++) {
            //printf("i=%d prefixitems\n", i);
            json_object *iobj = json_object_array_get_idx(jobj, i);
            json_object *ischema = json_object_array_get_idx(jprefixitems, i);

            char numstr[11];
            sprintf(numstr, "%d", i);
            json_object *jarrayitem_tmp_node = _jdac_output_create_node(numstr);

            int err = _jdac_validate_instance(iobj, ischema, jarrayitem_tmp_node);
            if (err) {
                _jdac_output_apply_result(jprefixitems_node, err);
                _jdac_output_append_node(jprefixitems_node, jarrayitem_tmp_node);
                prefixitems_ok = 0;
            } else {
                json_object_put(jarrayitem_tmp_node);
            }
        }
        int prefixitems_ret = (prefixitems_ok==1) ? JDAC_ERR_VALID : JDAC_ERR_INVALID;
        _jdac_output_apply_result(jprefixitems_node, prefixitems_ret);
    }

    if (jitems) {
        json_object *jitems_node = _jdac_output_create_and_append_node(joutput_node, "items");

        if (!json_object_is_type(jitems, json_type_object) &&
            !json_object_is_type(jitems, json_type_boolean)) {
                _jdac_output_apply_result(jitems_node, JDAC_ERR_SCHEMA_ERROR);
                return JDAC_ERR_SCHEMA_ERROR;
            }

        int jobj_arraylen = json_object_array_length(jobj);
        int items_arraylen = 0;
        for (int i=items_arraylen; i<jobj_arraylen; i++) {
            //printf("i=%d items\n", i);
            json_object *iobj = json_object_array_get_idx(jobj, i);
            char numstr[11];
            sprintf(numstr, "%d", i);
            json_object *jarrayitem_tmp_node = _jdac_output_create_node(numstr);
            int err = _jdac_validate_instance(iobj, jitems, jarrayitem_tmp_node);
            if (err) {
                _jdac_output_apply_result(jarrayitem_tmp_node, err);
                _jdac_output_append_node(jitems_node, jarrayitem_tmp_node);
                items_ok = 0;
            } else {
                json_object_put(jarrayitem_tmp_node);
            }
        }
        int items_ret = (items_ok==1) ? JDAC_ERR_VALID : JDAC_ERR_INVALID;
        _jdac_output_apply_result(jitems_node, items_ret);
    }
    int ret = (prefixitems_ok==1 && items_ok==1) ? JDAC_ERR_VALID : JDAC_ERR_INVALID;
    return ret;
}

json_object* _jdac_get_defs_from_ref(json_object* ref)
{
    char key[128];
    if (!json_object_is_type(ref, json_type_string))
        return NULL;

    const char *refstr = json_object_get_string(ref);
    if (sscanf(refstr, "#/$defs/%s", key)==1) {
        return json_object_object_get(defs, key);
    }
    return NULL;
}

int _jdac_value_is_equal(json_object *jobj1, json_object *jobj2)
{
    if (json_object_equal(jobj1, jobj2))
        return JDAC_ERR_VALID;

    if (json_object_is_type(jobj1, json_type_double) && json_object_is_type(jobj2, json_type_int)) {
        double value = json_object_get_double(jobj1);
        double value2 = json_object_get_int64(jobj2);
        if (value==round(value) && value == value2)
            return JDAC_ERR_VALID;
    }

    if (json_object_is_type(jobj1, json_type_int) && json_object_is_type(jobj2, json_type_double)) {
        double value = json_object_get_double(jobj2);
        double value2 = json_object_get_int64(jobj1);
        if (value==round(value) && value == value2)
            return JDAC_ERR_VALID;
    }

    return JDAC_ERR_INVALID;
}

int _jdac_check_const(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    json_object *jconst;
    int err = json_object_object_get_ex(jschema, "const", &jconst);
    if (err==0)
        return JDAC_ERR_VALID;

    err = _jdac_value_is_equal(jobj, jconst);
    if (err==JDAC_ERR_VALID)
        return JDAC_ERR_VALID;

    json_object *jnode = _jdac_output_create_and_append_node(joutput_node, "const");
    _jdac_output_apply_result(jnode, JDAC_ERR_INVALID);
    return JDAC_ERR_INVALID;
}

int _jdac_check_enums(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    json_object *jenum_array = json_object_object_get(jschema, "enum");

    if (!jenum_array)
        return JDAC_ERR_VALID;

    if (!json_object_is_type(jenum_array, json_type_array)) {
        json_object *jnode = _jdac_output_create_and_append_node(joutput_node, "enum");
        _jdac_output_apply_result(jnode, JDAC_ERR_SCHEMA_ERROR);
        return JDAC_ERR_SCHEMA_ERROR;
    }

    int arraylen = json_object_array_length(jenum_array);
    for (int i=0; i<arraylen; i++) {
        json_object *ienum = json_object_array_get_idx(jenum_array, i);
        int err = _jdac_value_is_equal(jobj, ienum);
        if (err==JDAC_ERR_VALID)
            return JDAC_ERR_VALID;
    }
    // printf("ERROR: enum check failed (%s not in enum)\n", json_object_to_json_string(jobj));

    json_object *jnode = _jdac_output_create_and_append_node(joutput_node, "enum");
    _jdac_output_apply_result(jnode, JDAC_ERR_INVALID);
    return JDAC_ERR_INVALID;
}

int _jdac_check_uniqueItems(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    json_object *juniq = json_object_object_get(jschema, "uniqueItems");
    if (!juniq) {
        return JDAC_ERR_VALID;
    }

    json_object *juniqueitems_node = _jdac_output_create_and_append_node(joutput_node, "uniqueItems");

    if (!json_object_is_type(juniq, json_type_boolean)) {
        _jdac_output_apply_result(juniqueitems_node, JDAC_ERR_SCHEMA_ERROR);
        return JDAC_ERR_SCHEMA_ERROR;
    }

    // uniqueItems=false is valid
    if (json_object_get_boolean(juniq)==0) {
        _jdac_output_apply_result(juniqueitems_node, JDAC_ERR_VALID);
        return JDAC_ERR_VALID;
    }

    int uniqueitems_ok = 1;
    int arraylen = json_object_array_length(jobj);
    for (int i=0; i<arraylen-1; i++) {
        json_object *iobj = json_object_array_get_idx(jobj, i);
        for (int j=i+1; j<arraylen; j++) {
        json_object *uobj = json_object_array_get_idx(jobj, j);
            if (json_object_equal(iobj, uobj)==1) {
                uniqueitems_ok = 0;
                char numstr[11];
                sprintf(numstr, "%d", i);
                json_object *jnotunique_node = _jdac_output_create_and_append_node(juniqueitems_node, numstr);
                _jdac_output_apply_result(jnotunique_node, JDAC_ERR_INVALID);
            }
        }
    }
    int ret = uniqueitems_ok == 1 ? JDAC_ERR_VALID : JDAC_ERR_INVALID;
    _jdac_output_apply_result(juniqueitems_node, ret);
    return ret;
}

int _jdac_check_maxmin_items(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    int err = JDAC_ERR_VALID;
    int arraylen = json_object_array_length(jobj);

    json_object *jmax = json_object_object_get(jschema, "maxItems");
    if (jmax) {
        if (json_object_is_type(jmax, json_type_int) ||
            json_object_is_type(jmax, json_type_double)
        ) {
            int maxitems = json_object_get_double(jmax);
            if (arraylen>maxitems) {
                json_object *jmaxitems_node = _jdac_output_create_and_append_node(joutput_node, "maxItems");
                _jdac_output_apply_result(jmaxitems_node, JDAC_ERR_INVALID);
                err = JDAC_ERR_INVALID;
            }
        }
    }

    json_object *jmin = json_object_object_get(jschema, "minItems");
    if (jmin) {
        if (json_object_is_type(jmin, json_type_int) ||
            json_object_is_type(jmin, json_type_double)
        ) {
            int minitems = json_object_get_double(jmin);
            if (arraylen<minitems) {
                json_object *jminitems_node = _jdac_output_create_and_append_node(joutput_node, "minItems");
                _jdac_output_apply_result(jminitems_node, JDAC_ERR_INVALID);
                err = JDAC_ERR_INVALID;
            }
        }
    }

    // if (err)
    //     printf("ERROR: failed at maxItems or minItems check\n");
    return err;
}

int _jdac_validate_array(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    int err;

    err = _jdac_check_prefixItems_and_items(jobj, jschema, joutput_node);
    if (err)
        return err;

    err = _jdac_check_uniqueItems(jobj, jschema, joutput_node);
    if (err)
        return err;

    err = _jdac_check_maxmin_items(jobj, jschema, joutput_node);
    if (err)
        return err;

#ifdef JDAC_CONTAINS
    err = _jdac_check_contains_and_minmaxcontains(jobj, jschema, joutput_node);
    if (err)
        return err;
#endif

    return JDAC_ERR_VALID;
}

int _jdac_validate_object(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    int err;
    if (defs==NULL)
        defs = json_object_object_get(jschema, "$defs");

    err = _jdac_check_required(jobj, jschema, joutput_node);
    if (err) return err;

    err = _jdac_check_properties(jobj, jschema, joutput_node);
    if (err) return err;

#ifdef JDAC_DEPENDENT
    err = _jdac_check_dependentrequired(jobj, jschema, joutput_node);
    if (err)
        return err;
#endif

#ifdef JDAC_PROPERTYNAMES
    err = _jdac_check_propertynames(jobj, jschema, joutput_node);
    if (err)
        return err;
#endif

#ifdef JDAC_PATTERNPROPERTIES
    err = _jdac_check_patternproperties(jobj, jschema, joutput_node);
    if (err)
        return err;
#endif

#ifdef JDAC_ADDITIONALPROPERTIES
    err = _jdac_check_additionalproperties(jobj, jschema, joutput_node);
    if (err)
        return err;
#endif

    return JDAC_ERR_VALID;
}

int utf8_length(const char *str)
{
    const char *pointer = str;
    int len = 0;
    while(pointer[0])
    {
        if ((pointer[0] & 0xC0) != 0x80)
            len++;
        pointer++;
    }
    return len;
}

int _jdac_validate_string(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    const char *str = json_object_get_string(jobj);
    //printf("strlen of %s %ld %d %d\n", str, strlen(str), json_object_get_string_len(jobj), utf8_length(str));

    int minlength_ok=1;
    json_object *jminlen = json_object_object_get(jschema, "minLength");
    if (jminlen) {
        int minlen = json_object_get_int64(jminlen);
        if (utf8_length(str)<minlen) {
            minlength_ok=0;
            json_object *jminlength_node = _jdac_output_create_and_append_node(joutput_node, "minLength");
            _jdac_output_apply_result(jminlength_node, JDAC_ERR_INVALID);
        }
    }

    int maxlength_ok=1;
    json_object *jmaxlen = json_object_object_get(jschema, "maxLength");
    if (jmaxlen) {
        int maxlen = json_object_get_int64(jmaxlen);
        if (utf8_length(str)>maxlen) {
            maxlength_ok=0;
            json_object *jmaxlength_node = _jdac_output_create_and_append_node(joutput_node, "maxLength");
            _jdac_output_apply_result(jmaxlength_node, JDAC_ERR_INVALID);
        }
    }

    int enums_ok=1;
    int err = _jdac_check_enums(jobj, jschema, joutput_node);
    if (err) {
        if (err==JDAC_ERR_SCHEMA_ERROR) {
            // _jdac_output_apply_result(joutput_node, JDAC_ERR_SCHEMA_ERROR);
            return err;
        }
        enums_ok=0;
    }

    int patterns_ok=1;
#ifdef JDAC_PATTERN
    err = _jdac_check_pattern(jobj, jschema, joutput_node);
    if (err) {
        if (err==JDAC_ERR_SCHEMA_ERROR) {
            // _jdac_output_apply_result(joutput_node, JDAC_ERR_SCHEMA_ERROR);
            return err;
        }
        patterns_ok=0;
    }
#endif

    int ret = minlength_ok==1 && maxlength_ok==1 && enums_ok==1 && patterns_ok==1 ? JDAC_ERR_VALID:JDAC_ERR_INVALID;
    // _jdac_output_apply_result(joutput_node, ret);
    return ret;
}

int _jdac_validate_integer(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    double value = (double)json_object_get_int64(jobj);
    int err = _jdac_validate_number(jobj, jschema, value, joutput_node);
    return err;
}

int _jdac_validate_double(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    double value = json_object_get_double(jobj);
    int err = _jdac_validate_number(jobj, jschema, value, joutput_node);
    return err;
}

int _jdac_validate_number(json_object *jobj, json_object *jschema, double value, json_object *joutput_node)
{
    int multipleOf_ok=1;
    json_object *jmult = json_object_object_get(jschema, "multipleOf");
    if (jmult) {
        double multipland = (double)json_object_get_double(jmult);
        if (multipland==0.0) {
            json_object *jmultipleOf_node = _jdac_output_create_and_append_node(joutput_node, "multipleOf");
            _jdac_output_apply_result(jmultipleOf_node, JDAC_ERR_SCHEMA_ERROR);
            // _jdac_output_apply_result(joutput_node, JDAC_ERR_SCHEMA_ERROR);
            return JDAC_ERR_SCHEMA_ERROR;
        }

        double divided = value/multipland;
        if (isinf(divided)!=0) {
            multipleOf_ok=0;
        }
        else if (divided != round(divided)) {
            multipleOf_ok=0;
        }
        if (multipleOf_ok==0) {
            json_object *jmultipleOf_node = _jdac_output_create_and_append_node(joutput_node, "multipleOf");
            _jdac_output_apply_result(jmultipleOf_node, JDAC_ERR_INVALID);
        }
    }

    int minimum_ok=1;
    json_object *jmin = json_object_object_get(jschema, "minimum");
    if (jmin) {
        double min = (double)json_object_get_double(jmin);
        if (value<min) {
            minimum_ok=0;
            json_object *jminimum_node = _jdac_output_create_and_append_node(joutput_node, "minimum");
            _jdac_output_apply_result(jminimum_node, JDAC_ERR_INVALID);
        }

    }

    int exclusiveMinimum_ok=1;
    json_object *jexclmin = json_object_object_get(jschema, "exclusiveMinimum");
    if (jexclmin) {
        double min = (double)json_object_get_double(jexclmin);
        if (value<=min) {
            exclusiveMinimum_ok=0;
            json_object *jexclusiveMinimum_node = _jdac_output_create_and_append_node(joutput_node, "exclusiveMinimum");
            _jdac_output_apply_result(jexclusiveMinimum_node, JDAC_ERR_INVALID);
        }
    }

    int maximum_ok=1;
    json_object *jmax = json_object_object_get(jschema, "maximum");
    if (jmax) {
        double max = (double)json_object_get_double(jmax);
        if (value>max) {
            maximum_ok=0;
            json_object *jmaximum_node = _jdac_output_create_and_append_node(joutput_node, "maximum");
            _jdac_output_apply_result(jmaximum_node, JDAC_ERR_INVALID);
        }
    }

    int exclusiveMaximum_ok=1;
    json_object *jexclmax = json_object_object_get(jschema, "exclusiveMaximum");
    if (jexclmax) {
        double max = (double)json_object_get_double(jexclmax);
        if (value>=max) {
            exclusiveMaximum_ok=0;
            json_object *jexclusiveMaximum_node = _jdac_output_create_and_append_node(joutput_node, "exclusiveMaximum");
            _jdac_output_apply_result(jexclusiveMaximum_node, JDAC_ERR_INVALID);
        }
    }
    int ret = multipleOf_ok==1 && minimum_ok==1 && exclusiveMinimum_ok==1 && maximum_ok==1 && exclusiveMaximum_ok==1 ? JDAC_ERR_VALID:JDAC_ERR_INVALID;
    // _jdac_output_apply_result(joutput_node, ret);
    return ret;
}

int _jdac_validate_boolean(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    // printf("%s\n", __func__);
    // _jdac_output_apply_result(joutput_node, JDAC_ERR_VALID);
    return JDAC_ERR_VALID;
}

int _jdac_validate_instance(json_object *jobj, json_object *jschema, json_object *joutput_node)
{
    int err;
    // printf("--validate instance--\n");
    // printf("%s\n", json_object_get_string(jobj));
    // printf("%s\n", json_object_get_string(jschema));

#ifdef JDAC_REF
    err = _jdac_check_ref(jobj, jschema, storagelist_head, joutput_node);
    if (err)
        return err;
#endif

    err = _jdac_check_bool(jobj, jschema, joutput_node);
    if (err)
        return err;

    err = _jdac_check_type(jobj, jschema, joutput_node);
    if (err)
        return err;

    err = _jdac_check_const(jobj, jschema, joutput_node);
    if (err)
        return err;

    err = _jdac_check_enums(jobj, jschema, joutput_node);
    if (err)
        return err;

#ifdef JDAC_SUBSCHEMALOGIC
    err = _jdac_check_subschemalogic(jobj, jschema, joutput_node);
    if (err)
        return err;
#endif

    json_type type = json_object_get_type(jobj);

    if (type==json_type_object)
        return _jdac_validate_object(jobj, jschema, joutput_node);
    else if (type==json_type_array)
        return _jdac_validate_array(jobj, jschema, joutput_node);
    else if (type==json_type_string)
        return _jdac_validate_string(jobj, jschema, joutput_node);
    else if (type==json_type_boolean)
        return _jdac_validate_boolean(jobj, jschema, joutput_node);
    else if (type==json_type_int)
        return _jdac_validate_integer(jobj, jschema, joutput_node);
    else if (type==json_type_double)
        return _jdac_validate_double(jobj, jschema, joutput_node);
    else if (type==json_type_null)
        return JDAC_ERR_VALID;
    else
        printf("%s: WARN: type %d not handled\n", __func__, type);

    return JDAC_ERR_VALID;
}

int jdac_validate(json_object *jobj, json_object *jschema)
{
#ifdef JDAC_STORE
    _jdac_store_traverse_json(&storagelist_head, jschema, NULL);
    _jdac_store_print(storagelist_head);
#endif

    json_object *joutput = _jdac_output_create_node("root");
    int err = _jdac_validate_instance(jobj, jschema, joutput);
    _jdac_output_apply_result(joutput, err);

    if (err)
        printf("Basic Output: %s\n", json_object_get_string(joutput));

#ifdef JDAC_STORE
    _jdac_store_free(&storagelist_head);
#endif
    json_object_put(joutput);
    return err;
}

int jdac_validate_file(const char *jsonfile, const char *jsonschemafile)
{
    int err = _jdac_load(jsonfile, jsonschemafile);
    if (err) return err;

    err = jdac_validate(json, schema);

    json_object_put(json);
    json_object_put(schema);
    json = NULL;
    schema = NULL;
    defs = NULL;
    return err;
}


#if JSON_C_MINOR_VERSION <= 12
#include <stdlib.h>
#include <stdbool.h>
int json_object_equal(struct json_object* jso1, struct json_object* jso2)
{
	if (jso1 == jso2)
		return 1;

	if (!jso1 || !jso2)
		return 0;

	if (json_object_get_type(jso1) != json_object_get_type(jso2))
		return 0;

    bool boolval1, boolval2;
    int intval1, intval2;
    double doubleval1, doubleval2;
    const char *string1, *string2;

	switch(json_object_get_type(jso1)) {
		case json_type_boolean:
            boolval1 = json_object_get_boolean(jso1);
            boolval2 = json_object_get_boolean(jso2);
            return (boolval1==boolval2);

		case json_type_double:
            doubleval1 = json_object_get_double(jso1);
            doubleval2 = json_object_get_double(jso2);
			return (doubleval1 == doubleval2);

		case json_type_int:
            intval1 = json_object_get_int(jso1);
            intval2 = json_object_get_int(jso2);
			return (intval1==intval2);

		case json_type_string:
            if (json_object_get_string_len(jso1)!=json_object_get_string_len(jso2))
                return 0;
            string1 = json_object_get_string(jso1);
            string2 = json_object_get_string(jso2);
			return (memcmp(string1,string2,json_object_get_string_len(jso1)) == 0);

		case json_type_object:
		// 	return json_object_all_values_equal(jso1, jso2);

		case json_type_array:
		// 	return json_array_equal(jso1, jso2);

		case json_type_null:
		return 1;
	};

	return 0;
}
#endif