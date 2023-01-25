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

int _jdac_check_ref(json_object *jobj, json_object *jschema, storage_node *storage_list)
{
// #ifdef JDAC_STORE
    json_object *jref = json_object_object_get(jschema, "$ref");
    if (jref) {
        const char *refstr = json_object_get_string(jref);
        printf("ref is %s\n", refstr);
        storage_node *rootnode = _jdac_store_get_root_node(storage_list);
        // if there is a rootnode id, compare it to the ref
        if (rootnode) {
            const char *path_ref = _jdac_uri_get_path(refstr);
            const char *path_id =  _jdac_uri_get_path(rootnode->id);

            if (!path_ref)
                return JDAC_ERR_VALID;

            if (!path_id) {
                char filepath[512];
                snprintf(filepath, sizeof(filepath)-1, "%s.%s", localpath, path_ref);
                printf("filepath is %s\n", filepath);
                json_object *jschemafromfile = json_object_from_file(filepath);
                if (!jschemafromfile)
                    return JDAC_ERR_JSON_NOT_FOUND;
                int err = _jdac_validate_instance(jobj, jschemafromfile);
                json_object_put(jschemafromfile);
                return err;
            }


            if (strcmp(path_ref, path_id)!=0 && strlen(localpath)>0) {
                printf("yep\n");
                char filepath[512];
                snprintf(filepath, sizeof(filepath)-1, "%s.%s", localpath, path_ref);
                printf("filepath is %s\n", filepath);
                json_object *jschemafromfile = json_object_from_file(filepath);
                if (!jschemafromfile)
                    return JDAC_ERR_JSON_NOT_FOUND;
                int err = _jdac_validate_instance(jobj, jschemafromfile);
                json_object_put(jschemafromfile);
                return err;
            }
            else if (strcmp(path_ref, path_id)==0) {
                int err = _jdac_validate_instance(jobj, rootnode->json_schema_ptr);
                return err;
            }
        }

        // if (refstr) {
        //     json_object *jschema_from_resolved_uri = _jdac_store_resolve(storage_list, refstr);
        //     if (!jschema_from_resolved_uri)
        //         return JDAC_ERR_INVALID_REF;
        //     int err = _jdac_validate_instance(jobj, jschema_from_resolved_uri);
        //     return err;
        // }
    }
// #endif
    return JDAC_ERR_VALID;
}