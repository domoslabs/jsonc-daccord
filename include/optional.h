#ifndef __OPTIONAL_H
#define __OPTIONAL_H

int _jdac_match_string_with_regex(const char* regex_pattern, const char* value);
int _jdac_check_pattern(json_object *jobj, json_object *jschema);
int _jdac_check_patternproperties(json_object *jobj, json_object *jschema);
int _jdac_check_additionalproperties(json_object *jobj, json_object *jschema);
int _jdac_check_propertynames(json_object *jobj, json_object *jschema);
int _jdac_check_subschemalogic(json_object *jobj, json_object *jschema);

#endif // __OPTIONAL_H
