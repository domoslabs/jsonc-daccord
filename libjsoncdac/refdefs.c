#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"


enum jdacfree {
    JDAC_IS_NOT_REMOTE = 0,
    JDAC_IS_REMOTE = 1
};

typedef struct reference_list {
    char reference_keyword[128];
    json_object *remoteSchema;
    int is_remote;
    struct reference_list *next;
}reference_list;

static json_object *rootschema = NULL;
static reference_list *reflist_head = NULL;

 void _jdac_reference_list_push(reference_list **head, const char *reference_keyword, json_object *jobj, int is_remote) {
    reference_list *new_node;
    new_node = malloc(sizeof(reference_list));
    strncpy(new_node->reference_keyword, reference_keyword, sizeof(new_node->reference_keyword));
    new_node->is_remote = is_remote>0 ? 1:0;
    new_node->next = *head;
    *head = new_node;
}

void _jdac_refdefs_init(json_object *jschema)
{
    if (rootschema)
        return;

    rootschema = jschema;
}

void _jdac_refdefs_close()
{
    rootschema=NULL;
}

int _jdac_is_numeric(const char *str)
{
    while(*str != '\0')
    {
        if(*str < '0' || *str > '9')
            return 0;
        str++;
    }
    return 1;
}

json_object* _jdac_refdefs_lookup(json_object *jschema)
{
    json_object *schema_ptr = rootschema;

    json_object *ref = json_object_object_get(jschema, "$ref");
    if (!ref)
        return NULL;

    const char *refstr = json_object_get_string(ref);
    if (!refstr)
        return NULL;

#ifdef JDAC_DOWNLOAD
    const char *has_url = _jdac_download_resolve(refstr);
    if (has_url) {
        char* dlschema = _jdac_download_schema(has_url);
        if (dlschema) {
            schema_ptr = json_tokener_parse(dlschema);
            free(dlschema);
            _jdac_reference_list_push(&reflist_head, refstr, schema_ptr, JDAC_IS_REMOTE);
            return schema_ptr;
        }
    }
#endif

    // strole the refs
    char keyword[128];
    keyword[0]=0;
    int len=0;
    const char *ptr = refstr;
    while(*ptr !=0 && len<sizeof(keyword)-1 && schema_ptr!=NULL )
    {
        keyword[len++]=*ptr;

        if ( ptr[0]=='~' && ptr[1]=='0') {
            keyword[len-1]='~';
            ptr++;
        }
        else if ( ptr[0]=='~' && ptr[1]=='1' ) {
            keyword[len-1]='/';
            ptr++;
        }
        else if ( ptr[1]!=0 && ptr[0]=='%' && ptr[1]=='2' && ptr[2]=='5' ) {
            keyword[len-1]='%';
            ptr++;
            ptr++;
        }
        keyword[len]=0;
        // printf("keyword: %s\n", keyword);

        if ( ptr[1]==0 || ptr[1]=='/' ) {
            keyword[len]=0;
            if (strncmp(keyword, "#", sizeof(keyword)-1)==0)
                schema_ptr = rootschema;
            else if (_jdac_is_numeric(keyword)==1) {
                if (!json_object_is_type(schema_ptr, json_type_array))
                    return NULL;
                schema_ptr = json_object_array_get_idx(schema_ptr, atoi(keyword));
            }
            else {
                schema_ptr= json_object_object_get(schema_ptr, keyword);
                // if (!json_object_is_type(jschema, json_type_null))
                //     printf("schema_ptr: %s\n", json_object_get_string(schema_ptr));
                // else
                //     printf("schema_ptr was null\n");
            }
            len=0;
        }
        ptr++;
        if (*ptr=='/')
            ptr++;
    }

    if (schema_ptr) {
        json_object *nestedref = json_object_object_get(schema_ptr, "$ref");
        if (nestedref) {
            return _jdac_refdefs_lookup(schema_ptr);
        }
    }
    _jdac_reference_list_push(&reflist_head, keyword, schema_ptr, JDAC_IS_NOT_REMOTE);

    return schema_ptr;
}
