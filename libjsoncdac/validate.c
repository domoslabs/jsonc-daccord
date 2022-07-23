#include <stdio.h>
#include <string.h>
#include <math.h>
#include <json-c/json.h>

#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

json_object *json = NULL;
json_object *schema = NULL;
json_object *defs = NULL;

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
    "INVALID ANYOF",
    "INVALID ENUMS",
    "INVALID STRING LENGTH",
    "INVALID UNIQUE ITEMS",
    "INVALID ARRAY LENGTH",
    "INVALID NUMBER",
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

int __jdac_inspect_type(json_object *jobj, const char *type)
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
        return JDAC_ERR_SCHEMA_ERROR;
    }
    return JDAC_ERR_INVALID_TYPE;
}

int _jdac_check_type(json_object *jobj, json_object *jschema)
{
        json_object *jtype = json_object_object_get(jschema, "type");

        if  (jtype==NULL) {
            return JDAC_ERR_VALID;
        }
        else if (json_object_is_type(jtype, json_type_string)) {
            const char *type = json_object_get_string(jtype);
            return __jdac_inspect_type(jobj, type);
        }
        else if (json_object_is_type(jtype, json_type_array)) {
            int arraylen = json_object_array_length(jtype);
            for (int i=0; i<arraylen; i++) {
                json_object *iobj = json_object_array_get_idx(jtype, i);
                if (!json_object_is_type(iobj, json_type_string))
                    return JDAC_ERR_SCHEMA_ERROR;
                const char *type = json_object_get_string(iobj);
                int err = __jdac_inspect_type(jobj, type);
                if (err==JDAC_ERR_VALID)
                    return JDAC_ERR_VALID;
            }
            return JDAC_ERR_INVALID_TYPE;
        }
        else
            return JDAC_ERR_SCHEMA_ERROR;
}


int _jdac_check_required(json_object *jobj, json_object *jschema)
{
    json_object *jarray = json_object_object_get(jschema, "required");
    int missing_required_key = 0;
    if (jarray) {
        int arraylen = json_object_array_length(jarray);
        for (int i = 0; i < arraylen; i++) {
            json_object *iobj = json_object_array_get_idx(jarray, i);
            const char *key = json_object_get_string(iobj);
            if (key) {
                //printf("%s\n", key);
                json_object *required_object = json_object_object_get(jobj, key);
                if (!required_object) {
                    printf("required key missing: %s\n", key);
                    missing_required_key=1;
                }
            }
        }
    }
    if (missing_required_key)
        return JDAC_ERR_INVALID_REQUIRED;
    else
        return JDAC_ERR_VALID;
}

int _jdac_check_properties(json_object *jobj, json_object *jschema)
{
    // printf("%s\n", __func__);

    json_object *jprops = json_object_object_get(jschema, "properties");
    if (jprops) {
        json_object_object_foreach(jprops, jprop_key, jprop_val) {
            // printf("key of prop is %s\n", jprop_key);
            json_object *iobj = json_object_object_get(jobj, jprop_key);
            //printf("iobj %s type %d\nkey %s\nval %s\n", json_object_get_string(iobj), json_object_get_type(iobj), jprop_key, json_object_get_string(jprop_val));
            if (iobj) {
                int err = jdac_validate_instance(iobj, jprop_val);
                if (err) return err;
            }
        }
    }
    return JDAC_ERR_VALID;
}

int _jdac_check_anyOf(json_object *jobj, json_object *jschema)
{
    //printf("%s\n", __func__);
    json_object *jschema_array = json_object_object_get(jschema, "anyOf");

    if (!jschema_array)
        return JDAC_ERR_VALID;

    int arraylen = json_object_array_length(jschema_array);
    for (int i = 0; i < arraylen; i++) {
        json_object *ischema = json_object_array_get_idx(jschema_array, i);
        if (!json_object_is_type(ischema, json_type_object)) {
            //printf("anyOf array item at idx %d is not an object\n", i);
            return JDAC_ERR_SCHEMA_ERROR;
        }
        int err = jdac_validate_instance(jobj, ischema); 
        if (err==JDAC_ERR_VALID)
            return JDAC_ERR_VALID;
    }
    return JDAC_ERR_INVALID_ANYOF;
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

int _jdac_check_items(json_object *jobj, json_object *jschema)
{
    int err;

    json_object *jitems = json_object_object_get(jschema, "items");
    if (jitems) {
            if (jitems) {
                int arraylen = json_object_array_length(jobj);
                for (int i=0; i<arraylen; i++) {
                    json_object *iobj = json_object_array_get_idx(jobj, i);
                    if (iobj) {
                        err = jdac_validate_instance(iobj, jitems);
                        if (err) return err;
                    }
                }
            }


        // json_object_object_foreach(jitems, jitem_key, jitem_val) {
        //     printf("key of item is %s\n", jitem_key);

        //     json_object *ischema = jitem_val;
        //     if (strcmp(jitem_key, "$ref")==0) {
        //         json_object *jdef =_jdac_get_defs_from_ref(jitem_val);
        //         if (jdef) {
        //             ischema = jdef;
        //         } else {
        //             printf("WARN: unable to find defs from ref (%s)\n", jitem_key);
        //             ischema = NULL;
        //         }
        //     }

        //     if (ischema) {
        //         int arraylen = json_object_array_length(jobj);
        //         for (int i=0; i<arraylen; i++) {
        //             json_object *iobj = json_object_array_get_idx(jobj, i);
        //             if (iobj) {
        //                 err = jdac_validate_instance(iobj, ischema);
        //                 if (err) return JDAC_ERR_INVALID;
        //             }
        //         }
        //     }
        // }
    }
    return JDAC_ERR_VALID;
}

int _jdac_check_enums(json_object *jobj, json_object *jschema)
{
    json_object *jenum_array = json_object_object_get(jschema, "enum");

    if (!jenum_array)
        return JDAC_ERR_VALID;

    if (!json_object_is_type(jenum_array, json_type_array))
        return JDAC_ERR_SCHEMA_ERROR;

    int arraylen = json_object_array_length(jenum_array);
    for (int i=0; i<arraylen; i++) {
        json_object *ienum = json_object_array_get_idx(jenum_array, i);
        if (json_object_equal(jobj, ienum))
            return JDAC_ERR_VALID;
    }
    printf("ERROR: enum check failed (%s not in enum)\n", json_object_to_json_string(jobj));

    return JDAC_ERR_INVALID_ENUMS;
}

int _jdac_check_uniqueItems(json_object *jobj, json_object *jschema)
{
    json_object *juniq = json_object_object_get(jschema, "uniqueItems");
    if (juniq) {
        if (!json_object_is_type(juniq, json_type_boolean))
            return JDAC_ERR_SCHEMA_ERROR;

        if (json_object_get_boolean(juniq)==0)
            return JDAC_ERR_VALID;

        int arraylen = json_object_array_length(jobj);
        for (int i=0; i<arraylen-1; i++) {
            json_object *iobj = json_object_array_get_idx(jobj, i);
            for (int j=i+1; j<arraylen; j++) {
            json_object *uobj = json_object_array_get_idx(jobj, j);
                if (json_object_equal(iobj, uobj)==1)
                    return JDAC_ERR_INVALID_UNIQUEITEMS;
            }
        }
    }
    return JDAC_ERR_VALID;
}

int _jdac_check_maxmin_items(json_object *jobj, json_object *jschema)
{
    int err = JDAC_ERR_VALID;
    json_object *jmax = json_object_object_get(jschema, "maxItems");
    json_object *jmin = json_object_object_get(jschema, "minItems");
    int arraylen = json_object_array_length(jobj);

    if (jmax) {
        if (json_object_is_type(jmax, json_type_int)) {
            int maxitems = json_object_get_int64(jmax);
            if (arraylen>maxitems)
                err = JDAC_ERR_INVALID_ARRAYLEN;
        }
    }

    if (jmin) {
        if (json_object_is_type(jmin, json_type_int)) {
            int minitems = json_object_get_int64(jmin);
            if (arraylen<minitems)
                err = JDAC_ERR_INVALID_ARRAYLEN;
        }
    }

    if (err)
        printf("ERROR: failed at maxItems or minItems check\n");
    return err;
}

int _jdac_validate_array(json_object *jobj, json_object *jschema)
{
    int err;

    err = _jdac_check_uniqueItems(jobj, jschema);
    if (err) return err;

    err = _jdac_check_maxmin_items(jobj, jschema);
    if (err) return err;

    err = _jdac_check_items(jobj, jschema);
    if (err) return err;

    return JDAC_ERR_VALID;
}

int _jdac_validate_object(json_object *jobj, json_object *jschema)
{
    int err;
    if (defs==NULL)
        defs = json_object_object_get(jschema, "$defs");

    err = _jdac_check_required(jobj, jschema);
    if (err) return err;

    err = _jdac_check_properties(jobj, jschema);
    if (err) return err;

#ifdef JDAC_PROPERTYNAMES
    err = _jdac_check_propertynames(jobj, jschema);
    if (err) return err;
#endif

#ifdef JDAC_PATTERNPROPERTIES
    err = _jdac_check_patternproperties(jobj, jschema);
    if (err) return err;
#endif

#ifdef JDAC_ADDITIONALPROPERTIES
    err = _jdac_check_additionalproperties(jobj, jschema);
    if (err) return err;
#endif

    err = _jdac_check_anyOf(jobj, jschema);
    if (err) return err;

    return JDAC_ERR_VALID;
}

int _jdac_validate_string(json_object *jobj, json_object *jschema)
{
    const char *str = json_object_get_string(jobj);
    json_object *jminlen = json_object_object_get(jschema, "minLength");
    if (jminlen) {
        int minlen = json_object_get_int64(jminlen);
        if (strlen(str)<minlen)
            return JDAC_ERR_INVALID_STRLEN;
    }
    json_object *jmaxlen = json_object_object_get(jschema, "maxLength");
    if (jmaxlen) {
        int maxlen = json_object_get_int64(jmaxlen);
        if (strlen(str)>maxlen)
            return JDAC_ERR_INVALID_STRLEN;
    }

    int err = _jdac_check_enums(jobj, jschema);
    if (err) return err;

#ifdef JDAC_PATTERN
    err = _jdac_check_pattern(jobj, jschema);
    if (err) return err;
#endif

    return JDAC_ERR_VALID;
}

int _jdac_validate_integer(json_object *jobj, json_object *jschema)
{
    double value = (double)json_object_get_int64(jobj);
    int err = _jdac_validate_number(jobj, jschema, value);
    return err;
}

int _jdac_validate_double(json_object *jobj, json_object *jschema)
{
    double value = json_object_get_double(jobj);
    int err = _jdac_validate_number(jobj, jschema, value);
    return err;
}

int _jdac_validate_number(json_object *jobj, json_object *jschema, double value)
{
    json_object *jmin = json_object_object_get(jschema, "minimum");
    if (jmin) {
        double min = (double)json_object_get_int64(jmin);
        if (value<min)
            return JDAC_ERR_INVALID_NUMBER;
    }
    json_object *jmax = json_object_object_get(jschema, "maximum");
    if (jmax) {
        double max = (double)json_object_get_int64(jmax);
        if (value>max)
            return JDAC_ERR_INVALID_NUMBER;
    }

    int err = _jdac_check_enums(jobj, jschema);
    if (err) return err;

    return JDAC_ERR_VALID;
}

int _jdac_validate_boolean(json_object *jobj, json_object *jschema)
{
    // printf("%s\n", __func__);
    // printf("%s\n", json_object_get_string(jobj));
    // printf("%s\n", json_object_get_string(jschema));
    return JDAC_ERR_VALID;
}

int jdac_validate_instance(json_object *jobj, json_object *jschema)
{
    // check if jschema is a bool, true or false
    if (json_object_is_type(jschema, json_type_boolean)) {
        json_bool value = json_object_get_boolean(jschema);
        if (value==0)
            return JDAC_ERR_INVALID;
        if (value==1)
            return JDAC_ERR_VALID;
    }

    int err = _jdac_check_type(jobj, jschema);
    if (err) return err;

    // if (!json_object_is_type(jobj, json_type_null))
    //     printf("%s\n", json_object_get_string(jobj));
    // else
    //     printf("jobj was null\n");
    // if (!json_object_is_type(jschema, json_type_null))
    //     printf("%s\n", json_object_get_string(jschema));
    // else
    //     printf("jschema was null\n");

    json_type type = json_object_get_type(jobj);

    if (type==json_type_object)
        return _jdac_validate_object(jobj, jschema);
    else if (type==json_type_array)
        return _jdac_validate_array(jobj, jschema);
    else if (type==json_type_string)
        return _jdac_validate_string(jobj, jschema);
    else if (type==json_type_boolean)
        return _jdac_validate_boolean(jobj, jschema);
    else if (type==json_type_int)
        return _jdac_validate_integer(jobj, jschema);
    else if (type==json_type_double)
        return _jdac_validate_double(jobj, jschema);
    else if (type==json_type_null)
        return JDAC_ERR_VALID;
    else
        printf("WARN: type %d not handled\n", type);

    return JDAC_ERR_VALID;
}

int jdac_validate(const char *jsonfile, const char *jsonschemafile)
{
    int err = _jdac_load(jsonfile, jsonschemafile);
    if (err) return err;

    err = jdac_validate_instance(json, schema);

    json_object_put(json);
    json_object_put(schema);
    json = NULL;
    schema = NULL;
    defs = NULL;
    return err;
}
