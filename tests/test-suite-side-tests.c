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

#define UNIT_TESTING 1  //overloads malloc,calloc,free,etc to mocka versions

// these tests are intended to be run along side the JSON Test Suite.

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

static void side_test_additionalProperties(void **state)
{
    json_object *jobj, *jschema;
    struct test_env *e = (struct test_env *)*state;
    e->json = json_object_from_file("json/side-test-patternProperties.json");
    e->schema = json_object_from_file("schema/side-test-patternProperties.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    // ignore null instance
    jschema = json_object_object_get(e->schema, "schema-valid");
    assert_non_null(jschema);
    assert_int_equal(jdac_validate_instance(NULL, jschema), JDAC_ERR_VALID);

    // invalid regex
    jobj = json_object_object_get(e->json, "botenanna");
    jschema = json_object_object_get(e->schema, "schema-invalid-regex");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(jdac_validate_instance(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    // non object patternProperties
    jobj = json_object_object_get(e->json, "botenanna");
    jschema = json_object_object_get(e->schema, "schema-non-object");
    assert_non_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(jdac_validate_instance(jobj, jschema), JDAC_ERR_SCHEMA_ERROR);

    // non object patternProperties
    jobj = json_object_object_get(e->json, "nonobject");
    jschema = json_object_object_get(e->schema, "schema-non-object");
    assert_null(jobj);
    assert_non_null(jschema);
    assert_int_equal(jdac_validate_instance(jobj, jschema), JDAC_ERR_VALID);

    freejson(state);
    freeschema(state);
}
int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(side_test_additionalProperties),
    };
    return cmocka_run_group_tests(tests, test_env_init, test_env_free);
}
