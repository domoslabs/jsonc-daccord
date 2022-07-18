#ifndef _DOMOS_TEST_COMMON_H
#define _DOMOS_TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define ASSERT(expr) \
    if (!(expr)){ \
        fprintf(stderr,"%s failed!\n", #expr); return 1; }

#define ASSERT_NUM(A, B) \
    if (A != B){ \
        fprintf(stderr,"%s: %"PRId64" != %"PRId64" \n", #A, (int64_t)A, (int64_t)B); return 1; }

#define ASSERT_STR(A, B) \
    if (strcmp(A, B) != 0){ \
        fprintf(stderr, "%s: %s != %s\n", #A, A, B); return 1; }

#endif // _DOMOS_TEST_COMMON_H