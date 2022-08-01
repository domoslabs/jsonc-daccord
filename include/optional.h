#ifndef __OPTIONAL_H
#define __OPTIONAL_H

#include <json-c/json.h>

int _jdac_match_string_with_regex(const char* regex_pattern, const char* value);
int _jdac_check_pattern(json_object *jobj, json_object *jschema);
int _jdac_check_patternproperties(json_object *jobj, json_object *jschema);
int _jdac_check_additionalproperties(json_object *jobj, json_object *jschema);
int _jdac_check_propertynames(json_object *jobj, json_object *jschema);
int _jdac_check_subschemalogic(json_object *jobj, json_object *jschema);
int _jdac_check_contains_and_minmaxcontains(json_object *jobj, json_object *jschema);
void _jdac_refdefs_init(json_object *jschema);
json_object* _jdac_refdefs_lookup(json_object *jschema);
void _jdac_refdefs_close();
char* _jdac_download_schema(const char *url);
const char* _jdac_download_resolve(const char *uri);


#endif // __OPTIONAL_H
