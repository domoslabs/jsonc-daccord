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
#include "../include/jdac_internal.h"

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

static void test_dependent_properties(void **state)
{
    json_object *jobj, *jschema;
    struct test_env *e = (struct test_env *)*state;
    e->json = json_object_from_file("json/test-dependent.json");
    e->schema = json_object_from_file("schema/test-dependent.json");
    assert_non_null(e->json);
    assert_non_null(e->schema);

    jobj = json_object_object_get(e->json, "test1");
    jschema = json_object_object_get(e->schema, "test1");
    assert_non_null(jobj);
    assert_non_null(jschema);
    int err = jdac_validate(jobj, jschema);
    printf("err %s\n", jdac_errorstr(err));
    assert_int_equal(err, JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test2");
    jschema = json_object_object_get(e->schema, "test2");
    assert_non_null(jobj);
    assert_non_null(jschema);
    err = jdac_validate(jobj, jschema);
    printf("err %s\n", jdac_errorstr(err));
    assert_int_equal(err, JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test3");
    jschema = json_object_object_get(e->schema, "test3");
    assert_non_null(jobj);
    assert_non_null(jschema);
    err = jdac_validate(jobj, jschema);
    printf("err %s\n", jdac_errorstr(err));
    assert_int_equal(err, JDAC_ERR_INVALID);

    jobj = json_object_object_get(e->json, "test4");
    jschema = json_object_object_get(e->schema, "test4");
    assert_non_null(jobj);
    assert_non_null(jschema);
    err = jdac_validate(jobj, jschema);
    printf("err %s\n", jdac_errorstr(err));
    assert_int_equal(err, JDAC_ERR_VALID);

    jobj = json_object_object_get(e->json, "test5");
    jschema = json_object_object_get(e->schema, "test5");
    assert_non_null(jobj);
    assert_non_null(jschema);
    err = jdac_validate(jobj, jschema);
    printf("err %s\n", jdac_errorstr(err));
    assert_int_equal(err, JDAC_ERR_VALID);

    freeschema(state);
}

int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_dependent_properties),
    };
    return cmocka_run_group_tests(tests, test_env_init, test_env_free);
}
