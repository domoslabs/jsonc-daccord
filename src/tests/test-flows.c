// cmocka includes
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <stdio.h>
#include "jsoncdaccord.h"

#define UNIT_TESTING 1  //overloads malloc,calloc,free,etc to mocka versions

static void test_validate_flows(void **state)
{
    assert_int_equal(jdac_validate("json/flows.json", "schema/flows.json"), JDAC_ERR_VALID);
}

int main()
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_validate_flows),

    };
    return cmocka_run_group_tests(tests, NULL, NULL); //move list init to setup and teardown
}