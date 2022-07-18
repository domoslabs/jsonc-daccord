#ifndef __JSONCDACCORD_H
#define __JSONCDACCORD_H

#include <json-c/json.h>

enum jdac_errors {
    JDAC_ERR_VALID = 0,
    JDAC_ERR_GENERAL_ERROR = -1,
    JDAC_ERR_JSON_NOT_FOUND = -2,
    JDAC_ERR_SCHEMA_NOT_FOUND = -3,
    JDAC_ERR_INVALID = -4,
    JDAC_ERR_WRONG_ARGS = -5
};

int jdac_validate(const char *jsonfile, const char *jsonschemafile);
int jdac_validate_node(json_object *jobj, json_object *jschema);


#endif //__JSONCDACCORD_H
