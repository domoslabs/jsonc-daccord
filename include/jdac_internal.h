#ifndef __JDAC_INTERNAL_H
#define __JDAC_INTERNAL_H

#include <json-c/json.h>

int _jdac_load(const char *jsonfile, const char *jsonschema);
int __jdac_inspect_type               (json_object *jobj, const char *type,     json_object *joutput_node);
int _jdac_check_type                  (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_check_required              (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_check_dependentrequired     (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_check_properties            (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_check_prefixItems_and_items (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_value_is_equal              (json_object *jobj1, json_object *jobj2);
int _jdac_check_const                 (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_check_enums                 (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_check_uniqueItems           (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_check_maxmin_items          (json_object *jobj, json_object *jschema, json_object *joutput_node); 
int _jdac_validate_array              (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_validate_object             (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_validate_string             (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_validate_integer            (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_validate_double             (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_validate_number             (json_object *jobj, json_object *jschema, double value, json_object *joutput_node);
int _jdac_validate_boolean            (json_object *jobj, json_object *jschema, json_object *joutput_node);
int _jdac_validate_instance           (json_object *jobj, json_object *jschema, json_object *joutput_node);


json_object* _jdac_output_create_node(const char *name);
void _jdac_output_append_node(json_object *joutput, json_object *jnode);
json_object* _jdac_output_create_and_append_node(json_object *joutput, const char *name);
json_object* _jdac_output_create_and_append_node_concatnames(json_object *joutput, char *name1, char *name2);
void _jdac_output_apply_result(json_object *joutput, enum jdac_errors err);
void _jdac_output_print_errors(json_object *joutput);

// a hack to get JSON_C version 0.12 to work
#if JSON_C_MINOR_VERSION <= 12
int json_object_equal(struct json_object* jso1, struct json_object* jso2);
#endif

#endif // __JDAC_INTERNAL_H
