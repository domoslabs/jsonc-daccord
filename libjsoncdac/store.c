#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
#include "../include/optional.h"

void _jdac_store_append(storage_node **head, storage_node *ref)
{
    storage_node *new_node = malloc(sizeof(storage_node));
    memcpy(new_node, ref, sizeof(storage_node));
    new_node->next = *head;
    *head = new_node;
}

void _jdac_store_free(storage_node **head)
{
    while(*head) {
        // if ((*head)->is_root==1 && (*head)->json_instance_ptr!=NULL)
        //     json_object_put((*head)->json_instance_ptr);
        storage_node *el = *head;
        *head = (*head)->next;
        free(el);
    }
    *head = NULL;
}

int _jdac_store_traverse_json(storage_node **head, json_object *jschema, char *pathbuffer)
{
    char pathbuf[256];
    pathbuf[0]=0;
    storage_node node = {0};

    if (pathbuffer==NULL) {
        strcpy(pathbuf, "#/");
        node.is_root = 1;
    }
    else {
        strcpy(pathbuf, pathbuffer);
        node.is_root = 0;
    }
    node.json_instance_ptr = jschema;

    json_object *jid = json_object_object_get(jschema, "$id");
    json_object *janchor = json_object_object_get(jschema, "$anchor");
    json_object *jdynamicanchor = json_object_object_get(jschema, "$dynamicAnchor");

    if (jid) {
        strcpy(node.id, json_object_get_string(jid));
    }

    if (janchor) {
        strcpy(node.anchor, json_object_get_string(janchor));
    }

    if (jdynamicanchor) {
        strcpy(node.dynamicAnchor, json_object_get_string(jdynamicanchor));
    }

    // if (jid || janchor || jdynamicanchor || node.is_root==1) {
        strcpy(node.JSONPtrURI, pathbuf);
        node.json_schema_ptr = jschema;
        _jdac_store_append(head, &node);
    // }

    json_object_object_foreach(jschema, jkey, jval) {
        if (json_object_is_type(jval, json_type_object)) {

            if (strcmp(jkey, "const")==0)
                continue;

            if (pathbuffer==NULL)
                sprintf(pathbuf, "#/%s", jkey);
            else
                sprintf(pathbuf, "%s/%s", pathbuffer, jkey);
            //printf("%s\n", pathbuf);
            _jdac_store_traverse_json(head, jval, pathbuf);
        }
    }
    return JDAC_ERR_VALID; 
}

void _jdac_store_print(storage_node *head)
{
    storage_node* list = head;
    printf("%-*s %-*s %-*s %-*s\n", 32, "JSONPtr", 16, "anchor", 16, "dynamicAnchor", 32, "id");
    while(list) {
        printf("%-*s %-*s %-*s %-*s\n", 32, list->JSONPtrURI, 16, list->anchor, 16, list->dynamicAnchor, 32, list->id);
        list = list->next;
    }
}

json_object* _jdac_store_resolve(storage_node *list, const char *uri)
{
    while(list) {
        if (strcmp(list->id, uri)==0)
            return list->json_instance_ptr;
        if (strcmp(list->JSONPtrURI, uri)==0)
            return list->json_instance_ptr;
        list = list->next;
    }

    return NULL;
}