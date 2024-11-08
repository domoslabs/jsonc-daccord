#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

char localpath[256] = {0};

int jdac_ref_set_localpath(const char *_localpath)
{
    strcpy(localpath, _localpath);
    return JDAC_ERR_VALID;
}

const char* _jdac_uri_get_path(const char *uri)
{
    const char *ptr = uri;
    char *schemaseparator = strstr(uri, "://");
    if (!schemaseparator)
    {
        //no schema

    } else {
        //has schema
        ptr = schemaseparator+3;
        char *path = strstr(ptr, "/");
        if (path)
            return path;
    }
    return NULL;
}

int _jdac_check_ref(json_object *jobj, json_object *jschema, storage_node *storage_list, json_object *joutput_node)
{
// #ifdef JDAC_STORE
    json_object *jref = json_object_object_get(jschema, "$ref");

    if (!jref) {
        return JDAC_ERR_VALID;
    }

    const char *refstr = json_object_get_string(jref);
    if (!refstr) {
        return JDAC_ERR_VALID;
    }

    // printf("ref is %s\n", refstr);

    json_object *jschema_from_resolved_uri = _jdac_store_resolve(storage_list, refstr);
    if (jschema_from_resolved_uri) {
        // printf("resolve schema: %s\n", json_object_get_string(jschema_from_resolved_uri));
        json_object *jref_node = _jdac_output_create_and_append_node(joutput_node, "$ref");
        int err = _jdac_validate_instance(jobj, jschema_from_resolved_uri, jref_node);
        _jdac_output_apply_result(jref_node, err);
        return err;
    }

    storage_node *rootnode = _jdac_store_get_root_node(storage_list);
    if (!rootnode)
        return JDAC_ERR_VALID;

    // if there is a rootnode id, compare it to the ref
    const char *path_ref = _jdac_uri_get_path(refstr);
    const char *path_id =  _jdac_uri_get_path(rootnode->id);

    if (!path_ref)
        return JDAC_ERR_VALID;

    json_object *jref_node = _jdac_output_create_and_append_node(joutput_node, "$ref");

    if (!path_id) {
        char filepath[512];
        snprintf(filepath, sizeof(filepath)-1, "%s.%s", localpath, path_ref);
        printf("filepath is %s\n", filepath);
        json_object *jschemafromfile = json_object_from_file(filepath);
        if (!jschemafromfile) {
            _jdac_output_apply_result(jref_node, JDAC_ERR_JSON_NOT_FOUND);
            return JDAC_ERR_JSON_NOT_FOUND;
        }
        int err = _jdac_validate_instance(jobj, jschemafromfile, jref_node);
        _jdac_output_apply_result(jref_node, err);
        json_object_put(jschemafromfile);
        return err;
    }


    if (strcmp(path_ref, path_id)!=0 && strlen(localpath)>0) {
        printf("yep\n");
        char filepath[512];
        snprintf(filepath, sizeof(filepath)-1, "%s.%s", localpath, path_ref);
        printf("filepath is %s\n", filepath);
        json_object *jschemafromfile = json_object_from_file(filepath);
        if (!jschemafromfile) {
            _jdac_output_apply_result(jref_node, JDAC_ERR_JSON_NOT_FOUND);
            return JDAC_ERR_JSON_NOT_FOUND;
        }
        int err = _jdac_validate_instance(jobj, jschemafromfile, jref_node);
        _jdac_output_apply_result(jref_node, err);
        json_object_put(jschemafromfile);
        return err;
    }
    else if (strcmp(path_ref, path_id)==0) {
        int err = _jdac_validate_instance(jobj, rootnode->json_schema_ptr, jref_node);
        _jdac_output_apply_result(jref_node, err);
        return err;
    }

    _jdac_output_apply_result(jref_node, JDAC_ERR_VALID);
    return JDAC_ERR_VALID;
}

