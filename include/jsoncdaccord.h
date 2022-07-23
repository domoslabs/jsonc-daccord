#ifndef __JSONCDACCORD_H
#define __JSONCDACCORD_H

#include <json-c/json.h>

enum jdac_errors {
    JDAC_ERR_VALID = 0,
    JDAC_ERR_GENERAL_ERROR,
    JDAC_ERR_JSON_NOT_FOUND,
    JDAC_ERR_SCHEMA_NOT_FOUND,
    JDAC_ERR_WRONG_ARGS,
    JDAC_ERR_SCHEMA_ERROR,
    JDAC_ERR_INVALID,
    JDAC_ERR_INVALID_TYPE,
    JDAC_ERR_INVALID_REQUIRED,
    JDAC_ERR_INVALID_ANYOF,
    JDAC_ERR_INVALID_ENUMS,
    JDAC_ERR_INVALID_STRLEN,
    JDAC_ERR_INVALID_UNIQUEITEMS,
    JDAC_ERR_INVALID_ARRAYLEN,
    JDAC_ERR_INVALID_NUMBER,
    JDAC_ERR_INVALID_PATTERNMATCH,
    JDAC_ERR_MAX
};

int jdac_validate(const char *jsonfile, const char *jsonschemafile);
int jdac_validate_instance(json_object *jobj, json_object *jschema);

const char* jdac_errorstr(unsigned int jdac_errors);

#endif //__JSONCDACCORD_H
