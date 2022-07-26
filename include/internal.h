#ifndef __INTERNAL_H
#define __INTERNAL_H

int _jdac_load(const char *jsonfile, const char *jsonschema);
int __jdac_inspect_type(json_object *jobj, const char *type);
int _jdac_check_type                  (json_object *jobj, json_object *jschema);
int _jdac_check_required              (json_object *jobj, json_object *jschema);
int _jdac_check_properties            (json_object *jobj, json_object *jschema);
int _jdac_check_prefixItems_and_items (json_object *jobj, json_object *jschema);
int _jdac_value_is_equal              (json_object *jobj1, json_object *jobj2);
int _jdac_check_const                 (json_object *jobj, json_object *jschema);
int _jdac_check_enums                 (json_object *jobj, json_object *jschema);
int _jdac_check_uniqueItems           (json_object *jobj, json_object *jschema);
int _jdac_check_maxmin_items          (json_object *jobj, json_object *jschema);
int _jdac_validate_array              (json_object *jobj, json_object *jschema);
int _jdac_validate_object             (json_object *jobj, json_object *jschema);
int _jdac_validate_string             (json_object *jobj, json_object *jschema);
int _jdac_validate_integer            (json_object *jobj, json_object *jschema);
int _jdac_validate_double             (json_object *jobj, json_object *jschema);
int _jdac_validate_number             (json_object *jobj, json_object *jschema, double value);
int _jdac_validate_boolean            (json_object *jobj, json_object *jschema);
int _jdac_validate_instance           (json_object *jobj, json_object *jschema);

#endif // __INTERNAL_H
