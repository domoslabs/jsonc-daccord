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
#include "jsoncdaccord.h"
//#include "common.h"

#define UNIT_TESTING 1  //overloads malloc,calloc,free,etc to mocka versions

char *testfile;

static void test_runner(void **state)
{
    json_object *jobj, *schema, *json;
    json = json_object_from_file(testfile);
    assert_non_null(json);
    assert_int_equal(json_object_is_type(json, json_type_array), 1);

    // cases
    int arraylen = json_object_array_length(json);
    for(int i=0; i<arraylen; i++) {
        json_object *iobj = json_object_array_get_idx(json, i);
        assert_non_null(iobj);
        json_object *jcasedescription = json_object_object_get(iobj, "description");
        json_object *jschema = json_object_object_get(iobj, "schema");
        json_object *jtests = json_object_object_get(iobj, "tests");
        assert_non_null(jcasedescription);
        assert_non_null(jschema);
        assert_non_null(jtests);
        assert_int_equal(json_object_is_type(jcasedescription, json_type_string), 1);
        assert_int_equal(json_object_is_type(jschema, json_type_object), 1);
        assert_int_equal(json_object_is_type(jtests, json_type_array), 1);

        printf("case: %s\n", json_object_get_string(jcasedescription));
        int arraylen2 = json_object_array_length(jtests);
        for(int j=0; j<arraylen2; j++) {
            json_object *jtest = json_object_array_get_idx(jtests, j);
            assert_int_equal(json_object_is_type(jtest, json_type_object), 1);
            json_object *jtestdescription = json_object_object_get(jtest, "description");
            json_object *jdata = json_object_object_get(jtest, "data");
            json_object *jvalid = json_object_object_get(jtest, "valid");
            assert_non_null(jtestdescription);
            printf("    test: %s\n", json_object_get_string(jtestdescription));
            assert_non_null(jdata);
            assert_non_null(jvalid);
            assert_int_equal(json_object_is_type(jvalid, json_type_boolean), 1);
            json_bool valid = json_object_get_boolean(jvalid);
            if (valid==1)
                assert_int_equal(jdac_validate_node(jdata, jschema), JDAC_ERR_VALID);
            else
                assert_int_not_equal(jdac_validate_node(jdata, jschema), JDAC_ERR_VALID);
        }
    }


}

int main(int argc, char *argv[])
{
    testfile = argv[1];
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_runner),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
