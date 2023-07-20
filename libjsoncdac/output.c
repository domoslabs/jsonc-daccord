#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

#ifdef JDAC_ERROR_OUTPUT

json_object* _jdac_output_create_node(const char *name)
{
    json_object *joutput = json_object_new_object();
    json_object_object_add(joutput, "name", json_object_new_string(name));
    json_object_object_add(joutput, "nodes", json_object_new_array());
    return joutput;
}

void _jdac_output_append_node(json_object *joutput, json_object *jnode)
{
    json_object *jarray;
    if (json_object_object_get_ex(joutput, "nodes", &jarray)) {
        json_object_array_add(jarray, jnode);
    }
}

json_object* _jdac_output_create_and_append_node(json_object *joutput, const char *name)
{
    json_object *jarray;
    if (json_object_object_get_ex(joutput, "nodes", &jarray)) {
        json_object *jnode_new = _jdac_output_create_node(name);
        json_object_array_add(jarray, jnode_new);
        return jnode_new;
    }
    return NULL;
}

json_object* _jdac_output_create_and_append_node_concatnames(json_object *joutput, char *name1, char *name2)
{
    char newname[256];
    snprintf(newname, sizeof(newname), "%s/%s", name1, name2);
    return _jdac_output_create_and_append_node(joutput, newname);
}


void _jdac_output_apply_result(json_object *joutput, enum jdac_errors err)
{
    int res = err==JDAC_ERR_VALID ? 1:0;
    json_object_object_add(joutput, "valid", json_object_new_boolean(res));
}

void __jdac_output_print_traverse_error_nodes(json_object *jnode, const char *parentpathstr) 
{
    char path[256];
    if (jnode==NULL) {
        return;
    }

    json_object *jobj;
    if (json_object_object_get_ex(jnode, "valid", &jobj)) {
        int valid = json_object_get_boolean(jobj);
        if (valid) {
            return;
        }
    }

    json_object *jarray;
    if (json_object_object_get_ex(jnode, "nodes", &jarray)) {
        int arraylen = json_object_array_length(jarray);
        if (arraylen==0) {
            printf("ERROR: %s\n", parentpathstr);
        }
        for (int i=0; i<arraylen; i++) {
            json_object *jnode = json_object_array_get_idx(jarray, i);
            json_object *jname;
            if (json_object_object_get_ex(jnode, "name", &jname)) {
                snprintf(path, 256, "%s/%s", parentpathstr, json_object_get_string(jname));
                __jdac_output_print_traverse_error_nodes(jnode, path);
            }
        }
    }
}

void _jdac_output_print_errors(json_object *joutput)
{
    if (!joutput) {
        return;
    }

    json_object *rootnode = joutput;;
    json_object *jobj;
    if (json_object_object_get_ex(rootnode, "valid", &jobj)) {
        int valid = json_object_get_boolean(jobj);
        if (valid) {
            //printf("Validation OK\n");
        } else {
            json_object *jarray;
            if (json_object_object_get_ex(rootnode, "nodes", &jarray)) {
                int arraylen = json_object_array_length(jarray);
                for (int i=0; i<arraylen; i++) {
                    json_object *jnode = json_object_array_get_idx(jarray, i);
                    __jdac_output_print_traverse_error_nodes(jnode, "#");
                }
            }
        }
    }
}

#else
json_object* _jdac_output_create_node(const char *name)
{
    return NULL;
}

void _jdac_output_append_node(json_object *joutput, json_object *jnode)
{
}

json_object* _jdac_output_create_and_append_node(json_object *joutput, const char *name)
{
    return NULL;
}

json_object* _jdac_output_create_and_append_node_concatnames(json_object *joutput, char *name1, char *name2)
{
    return NULL;
}


void _jdac_output_apply_result(json_object *joutput, enum jdac_errors err)
{
}

void _jdac_output_print_errors(json_object *joutput)
{    
}

#endif