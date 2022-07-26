// cmocka includes
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <json-c/json.h>
#include "../include/jsoncdaccord.h"
#include "../include/internal.h"
//#include "common.h"

#define UNIT_TESTING 1  //overloads malloc,calloc,free,etc to mocka versions

struct test_env{
    struct json_object *json;
    struct json_object *schema;
};

static int test_env_init(void **state){
    printf("%s\n", __func__);
    struct test_env *env = (struct test_env *) malloc(sizeof(struct test_env));
    *state = env;
    memset(env, 0, sizeof(struct test_env));

    return 0;
}

// these are needed when a test fails, we don't want address-sanitiser to thing there is a memory error.
void freejson(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    if (e->json != NULL) {
        json_object_put(e->json);
        e->json = NULL;
    }
}

void freeschema(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    if (e->schema != NULL) {
        json_object_put(e->schema);
        e->schema = NULL;
    }
}

static int test_env_free(void **state){
    printf("%s\n", __func__);
    struct test_env *env =  (struct test_env *) *state;

    freejson(state);
    freeschema(state);

    free(env);
    return 0;
}

enum willreturn {
    WILL_RETURN_NULL = 0,
    WILL_RETURN_NON_NULL = 1,
    WILL_RETURN_USE_REAL_FUNCTION = 2
};

struct json_object *__real_json_object_from_file(const char *filename);
struct json_object *__wrap_json_object_from_file(const char *filename)
{
    //printf("__wrap_json_object_from_file()\n");
    int i = mock_type(int);
    if (i==WILL_RETURN_NULL)
        return NULL;
    if (i==WILL_RETURN_NON_NULL)
        return json_object_new_object();
    if (i==WILL_RETURN_USE_REAL_FUNCTION)
        return __real_json_object_from_file(filename);

    return NULL;
}

static void test_jdac_load(void **state)
{
    will_return(__wrap_json_object_from_file, WILL_RETURN_NULL);
    assert_int_equal(_jdac_load("json", "schema"), JDAC_ERR_JSON_NOT_FOUND);

    will_return(__wrap_json_object_from_file, WILL_RETURN_NON_NULL);
    will_return(__wrap_json_object_from_file, WILL_RETURN_NULL);
    assert_int_equal(_jdac_load("json", "schema"), JDAC_ERR_SCHEMA_NOT_FOUND);

    will_return(__wrap_json_object_from_file, WILL_RETURN_NON_NULL);
    will_return(__wrap_json_object_from_file, WILL_RETURN_NON_NULL);
    assert_int_equal(_jdac_load("json", "schema"), JDAC_ERR_VALID);
}

static void test_check_type(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj, *jschema;

    e->json = __real_json_object_from_file("json/test-type.json");
    e->schema = __real_json_object_from_file("schema/test-type.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jobj = json_object_object_get(e->json, "test-object");
    jschema  = json_object_object_get(e->schema, "test-nonstring");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    jobj = json_object_object_get(e->json, "test-object");
    jschema  = json_object_object_get(e->schema, "test-nonstring2");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    jobj = json_object_object_get(e->json, "test-object");
    jschema  = json_object_object_get(e->schema, "test-object");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test-array");
    jschema  = json_object_object_get(e->schema, "test-object");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_INVALID_TYPE);
    jobj = json_object_object_get(e->json, "test-object");
    jschema  = json_object_object_get(e->schema, "test-array");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    jobj = json_object_object_get(e->json, "test-object");
    jschema  = json_object_object_get(e->schema, "test-object");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test-array");
    jschema  = json_object_object_get(e->schema, "test-array");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test-string");
    jschema  = json_object_object_get(e->schema, "test-string");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-string");
    jschema  = json_object_object_get(e->schema, "test-integer");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_INVALID_TYPE);
    jobj = json_object_object_get(e->json, "test-integer");
    jschema  = json_object_object_get(e->schema, "test-string");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    jobj = json_object_object_get(e->json, "test-integer");
    jschema  = json_object_object_get(e->schema, "test-integer");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test-double");
    jschema  = json_object_object_get(e->schema, "test-integer");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    jobj = json_object_object_get(e->json, "test-double");
    jschema  = json_object_object_get(e->schema, "test-integer");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    jobj = json_object_object_get(e->json, "test-double");
    jschema  = json_object_object_get(e->schema, "test-double");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test-integer");
    jschema  = json_object_object_get(e->schema, "test-double");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    jobj = json_object_object_get(e->json, "test-double");
    jschema  = json_object_object_get(e->schema, "test-string");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    jobj = json_object_object_get(e->json, "test-boolean");
    jschema  = json_object_object_get(e->schema, "test-boolean");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test-string");
    jschema  = json_object_object_get(e->schema, "test-boolean");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    jschema  = json_object_object_get(e->schema, "test-nonsense");
    assert_non_null(jschema);
    assert_int_equal(_jdac_check_type(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);



    freejson(state);
    freeschema(state);
}

static void test_check_required(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj, *jschema;

    e->json = __real_json_object_from_file("json/test-required.json");
    e->schema = __real_json_object_from_file("schema/test-required.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    // an empty schema shall always result in true
    jschema  = json_object_object_get(e->schema, "test-emptyschema");
    assert_non_null(jschema);
 
    jobj = json_object_object_get(e->json, "test-emptyjson");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-abc");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-a");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-wrongtype");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);

    jschema  = json_object_object_get(e->schema, "test-norequired");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "test-emptyjson");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-abc");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-a");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);

    jschema  = json_object_object_get(e->schema, "test-required-empty");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "test-emptyjson");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-abc");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-a");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-wrongtype");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);

    jschema  = json_object_object_get(e->schema, "test-require-a");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "test-keywords-abc");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-ab");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-a");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-bc");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_INVALID_REQUIRED);
    jobj = json_object_object_get(e->json, "test-wrongtype");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_INVALID_REQUIRED);
    jobj = json_object_object_get(e->json, "test-emptyjson");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_INVALID_REQUIRED);

    jschema  = json_object_object_get(e->schema, "test-require-abc");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "test-keywords-abc");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "test-keywords-ab");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_INVALID_REQUIRED);
    jobj = json_object_object_get(e->json, "test-keywords-a");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_INVALID_REQUIRED);
    jobj = json_object_object_get(e->json, "test-keywords-bc");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_INVALID_REQUIRED);
    jobj = json_object_object_get(e->json, "test-wrongtype");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_INVALID_REQUIRED);
    jobj = json_object_object_get(e->json, "test-emptyjson");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_required(jobj, jschema), JDAC_ERR_INVALID_REQUIRED);
    freejson(state);
    freeschema(state);
}

static void test_check_properties(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj, *jschema;

    e->json = __real_json_object_from_file("json/test-properties.json");
    e->schema = __real_json_object_from_file("schema/test-properties.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jschema  = json_object_object_get(e->schema, "noproperties");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "emptyobject");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjson");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjsontrue");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjsonfalse");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);

    jschema  = json_object_object_get(e->schema, "emptyproperties");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "emptyobject");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjson");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjsontrue");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjsonfalse");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);

    jschema  = json_object_object_get(e->schema, "typetests");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "emptyobject");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjson");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjsontrue");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjsonfalse");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    jschema  = json_object_object_get(e->schema, "string-properties");
    assert_non_null(jschema);
    jobj = json_object_object_get(e->json, "testjsonstring-all");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjsonstring-all-fail");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_INVALID_STRLEN);
    jobj = json_object_object_get(e->json, "testjsonstring2");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_INVALID_STRLEN);
    jobj = json_object_object_get(e->json, "testjsonstring3");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjsonstring4");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "testjsonstring5");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_properties(jobj, jschema), JDAC_ERR_INVALID_STRLEN);
    freejson(state);
    freeschema(state);
}

static void test_check_items(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj, *jschema;

    e->json = __real_json_object_from_file("json/test-items.json");
    e->schema = __real_json_object_from_file("schema/test-items.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jschema  = json_object_object_get(e->schema, "test1");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "test1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "wrongtype");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    jobj = json_object_object_get(e->json, "missingkey");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_REQUIRED);

    jschema  = json_object_object_get(e->schema, "nested");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "nestedarray");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "nestedarrayofstrings");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_TYPE);

    freejson(state);
    freeschema(state);
}

static void test_check_enums(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj, *jschema;

    e->json = __real_json_object_from_file("json/test-enum.json");
    e->schema = __real_json_object_from_file("schema/test-enum.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);


    jschema  = json_object_object_get(e->schema, "enum-string");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "teststring1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "teststring2");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "teststring3");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "teststring4");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_ENUMS);


    jschema  = json_object_object_get(e->schema, "enum-integer");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "testinteger1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "testinteger2");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "testinteger3");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "testinteger4");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_ENUMS);


    jschema  = json_object_object_get(e->schema, "enum-schemaerror");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "testinteger1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    jobj = json_object_object_get(e->json, "testinteger2");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    jobj = json_object_object_get(e->json, "testinteger3");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    jobj = json_object_object_get(e->json, "testinteger4");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    freejson(state);
    freeschema(state);
}


static void test_check_uniqueItems(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj, *jschema;

    e->json = __real_json_object_from_file("json/test-uniqueitems.json");
    e->schema = __real_json_object_from_file("schema/test-uniqueitems.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jschema  = json_object_object_get(e->schema, "unique");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "array-empty");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-unique1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-unique2");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-notunique1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_UNIQUEITEMS);

    jobj = json_object_object_get(e->json, "array-notunique2");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_UNIQUEITEMS);

    jobj = json_object_object_get(e->json, "array-notunique3");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_UNIQUEITEMS);

    jobj = json_object_object_get(e->json, "array-notunique4");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_UNIQUEITEMS);

    jobj = json_object_object_get(e->json, "array-notunique5");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_UNIQUEITEMS);

    jschema  = json_object_object_get(e->schema, "notunique");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "array-empty");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-unique1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-unique2");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-notunique1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-notunique2");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-notunique3");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-notunique4");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "array-notunique5");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jschema  = json_object_object_get(e->schema, "schemaerror");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "array-notunique1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    jobj = json_object_object_get(e->json, "array-notunique2");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    freejson(state);
    freeschema(state);
}

static void test_check_maxmin_items(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj, *jschema;

    e->json = __real_json_object_from_file("json/test-maxmin-items.json");
    e->schema = __real_json_object_from_file("schema/test-maxmin-items.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jschema  = json_object_object_get(e->schema, "schema-2-3");
    assert_non_null(jschema);
    jobj = json_object_object_get(e->json, "array-1");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_maxmin_items(jobj, jschema), JDAC_ERR_INVALID_ARRAYLEN);
    jobj = json_object_object_get(e->json, "array-2");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_maxmin_items(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "array-3");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_maxmin_items(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "array-4");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_maxmin_items(jobj, jschema), JDAC_ERR_INVALID_ARRAYLEN);
    jobj = json_object_object_get(e->json, "array-5");
    assert_non_null(jobj);
    assert_int_equal(_jdac_check_maxmin_items(jobj, jschema), JDAC_ERR_INVALID_ARRAYLEN);
    freejson(state);
    freeschema(state);
}

static void test_validate_array(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj, *jschema;

    e->json = __real_json_object_from_file("json/test-maxmin-items.json");
    e->schema = __real_json_object_from_file("schema/test-maxmin-items.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jschema  = json_object_object_get(e->schema, "schema-2-3");
    assert_non_null(jschema);
    jobj = json_object_object_get(e->json, "array-1");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_ARRAYLEN);
    jobj = json_object_object_get(e->json, "array-2");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "array-3");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);
    jobj = json_object_object_get(e->json, "array-4");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_ARRAYLEN);
    jobj = json_object_object_get(e->json, "array-5");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_ARRAYLEN);
    freejson(state);
    freeschema(state);
}

static void test_validate_number(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    e->schema = json_object_new_object();
    assert_non_null(e->schema);

    // Schema: type integer
    json_object_object_add(e->schema, "type", json_object_new_string("integer"));

    e->json = json_object_new_int(5);
    assert_non_null(e->json);
    assert_int_equal(_jdac_validate_integer(e->json, e->schema), JDAC_ERR_VALID);

    json_object_object_add(e->schema, "minimum", json_object_new_int(4));
    json_object_object_add(e->schema, "maximum", json_object_new_int(6));
    assert_int_equal(_jdac_validate_integer(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);

    e->json = json_object_new_int(3);
    assert_int_equal(_jdac_validate_integer(e->json, e->schema), JDAC_ERR_INVALID_NUMBER);
    freejson(state);
    e->json = json_object_new_int(4);
    assert_int_equal(_jdac_validate_integer(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_int(5);
    assert_int_equal(_jdac_validate_integer(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_int(6);
    assert_int_equal(_jdac_validate_integer(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_int(7);
    assert_int_equal(_jdac_validate_integer(e->json, e->schema), JDAC_ERR_INVALID_NUMBER);
    freejson(state);

    e->json = json_object_new_int(3);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_INVALID_NUMBER);
    freejson(state);
    e->json = json_object_new_int(4);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_int(5);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_int(6);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_int(7);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_INVALID_NUMBER);
    freejson(state);


    // Schema: type double
    freeschema(state);
    e->schema = json_object_new_object();
    json_object_object_add(e->schema, "type", json_object_new_string("double"));

    e->json = json_object_new_int(5);
    assert_non_null(e->json);
    assert_int_equal(_jdac_validate_double(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);

    e->json = json_object_new_double(5.0);
    assert_non_null(e->json);
    assert_int_equal(_jdac_validate_double(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);

    e->json = json_object_new_double(5.0);
    json_object_object_add(e->schema, "minimum", json_object_new_double(4.0));
    json_object_object_add(e->schema, "maximum", json_object_new_double(6.0));
    assert_int_equal(_jdac_validate_double(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);

    e->json = json_object_new_double(3.0);
    assert_int_equal(_jdac_validate_double(e->json, e->schema), JDAC_ERR_INVALID_NUMBER);
    freejson(state);
    e->json = json_object_new_double(4-0);
    assert_int_equal(_jdac_validate_double(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_double(5.0);
    assert_int_equal(_jdac_validate_double(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_double(6.0);
    assert_int_equal(_jdac_validate_double(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_double(7.0);
    assert_int_equal(_jdac_validate_double(e->json, e->schema), JDAC_ERR_INVALID_NUMBER);
    freejson(state);

    e->json = json_object_new_double(3.0);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_INVALID_NUMBER);
    freejson(state);
    e->json = json_object_new_double(4-0);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_double(5.0);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_double(6.0);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);
    e->json = json_object_new_double(7.0);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_INVALID_NUMBER);
    freejson(state);

    freejson(state);
    freeschema(state);
}

static void test_check_anyof(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj, *jschema;

    e->json = __real_json_object_from_file("json/test-anyof.json");
    e->schema = __real_json_object_from_file("schema/test-anyof.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jschema = json_object_object_get(e->schema, "acceptstringandinteger");
    assert_non_null(jschema);

    jobj = json_object_object_get(e->json, "test-int");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test-double");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_SUBSCHEMALOGIC);

    jobj = json_object_object_get(e->json, "test-string");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test-boolean");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_INVALID_SUBSCHEMALOGIC);

    jschema = json_object_object_get(e->schema, "anyOfSchemaError");
    assert_non_null(jschema);
    jobj = json_object_object_get(e->json, "test-int");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    freejson(state);
    freeschema(state);
}

static void test_requirements_recursively(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj;

    e->json = __real_json_object_from_file("json/test-reqs-recursively.json");
    e->schema = __real_json_object_from_file("schema/test-reqs-recursively.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jobj = json_object_object_get(e->json, "alltrue");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, e->schema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "nametooshort");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, e->schema), JDAC_ERR_INVALID_STRLEN);

    jobj = json_object_object_get(e->json, "wrongtype");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, e->schema), JDAC_ERR_INVALID_TYPE);

    jobj = json_object_object_get(e->json, "husbandmissing");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, e->schema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "namemissing");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, e->schema), JDAC_ERR_INVALID_REQUIRED);

    jobj = json_object_object_get(e->json, "empty");
    assert_non_null(jobj);
    assert_int_not_equal(jdac_validate(jobj, e->schema), JDAC_ERR_VALID);

    freejson(state);
    freeschema(state);
}

static void test_the_vectors_json(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    e->json = __real_json_object_from_file("json/vectors.json");
    e->schema = __real_json_object_from_file("schema/vectors.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_VALID);
    freejson(state);

    e->json = __real_json_object_from_file("json/vectors2.json");
    assert_non_null(e->json);
    assert_int_equal(jdac_validate(e->json, e->schema), JDAC_ERR_INVALID_REQUIRED);

    freejson(state);
    freeschema(state);
}

static void test_the_vectors_variants(void **state)
{
    struct test_env *e = (struct test_env *)*state;
    json_object *jobj;
    e->json = __real_json_object_from_file("json/vectors-variants.json");
    e->schema = __real_json_object_from_file("schema/vectors.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jobj = json_object_object_get(e->json, "truejson");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, e->schema), JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "validport_toohigh");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, e->schema), JDAC_ERR_INVALID_NUMBER);

    jobj = json_object_object_get(e->json, "ignore_indices_index_too_high");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, e->schema), JDAC_ERR_INVALID_NUMBER);

    jobj = json_object_object_get(e->json, "too_many_vector_elements");
    assert_non_null(jobj);
    assert_int_equal(jdac_validate(jobj, e->schema), JDAC_ERR_INVALID_ARRAYLEN);
    freejson(state);
    freeschema(state);

}
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_jdac_load),
        cmocka_unit_test(test_check_type),
        cmocka_unit_test(test_check_required),
        cmocka_unit_test(test_check_properties),
        cmocka_unit_test(test_check_items),
        cmocka_unit_test(test_check_enums),
        cmocka_unit_test(test_check_uniqueItems),
        cmocka_unit_test(test_check_maxmin_items),
        cmocka_unit_test(test_validate_array),
        cmocka_unit_test(test_validate_number),
        cmocka_unit_test(test_check_anyof),
        cmocka_unit_test(test_requirements_recursively),
        cmocka_unit_test(test_the_vectors_json),
        cmocka_unit_test(test_the_vectors_variants),
    };
    return cmocka_run_group_tests(tests, test_env_init, test_env_free);
}
