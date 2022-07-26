#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

static json_object *rootschema = NULL;

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

void _jdac_refdefs_decode_ref(const char *refstr, char *decoded, int bufsize)
{
    const char *ptr = refstr;
    int len=0;
    decoded[0]=0;
    while(*ptr && len<bufsize-1) {
        if (ptr[0]=='~' && ptr[1]=='0') {
            decoded[len++] = '~';
            ptr++;
        }
        else if (ptr[0]=='~' && ptr[1]=='1') {
            decoded[len++] = '/';
            ptr++;
        }
        else if (ptr[0]=='%') {
            if ( (ptr[1] <= '9' && ptr[1] >= 0) && ((ptr[2] <= '9' && ptr[2] >= 0))) {
                char digits[3];
                strncpy(digits, &ptr[1], 2);
                ptr++;
                ptr++;
                if (_jdac_is_numeric(digits)) {
                    if (atoi(digits)==25) {
                        decoded[len++] = '%';
                    }
                }
            }
        }
        else
            decoded[len++] = *ptr;
        ptr++;
    }
    decoded[len] = 0;

}

json_object* _jdac_refdefs_lookup(json_object *jschema)
{
    json_object *ref = json_object_object_get(jschema, "$ref");
    if (!ref)
        return NULL;

    const char *refstr = json_object_get_string(ref);
    if (!refstr)
        return NULL;

    // strole the refs
    char keyword[128];
    keyword[0]=0;
    int len=0;
    const char *ptr = refstr;
    json_object *schema_ptr = rootschema;
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
        if (nestedref)
            return _jdac_refdefs_lookup(schema_ptr);
    }

    return schema_ptr;
}
