#include <stdio.h>
#include "include/jsoncdaccord.h"
#include "version_config.h"

void usage()
{
    printf("jdac-cli (%s version %s)\n", PROJECT_NAME, PROJECT_VER);
    printf("\nUsage:\n");
    printf("    jdac-cli path-to-json path-to-schema\n\n");
}

int main(int argc, char *argv[])
{
    if (argc!=3) {
        usage();
        return JDAC_ERR_WRONG_ARGS;
    }
    printf("validating %s with %s\n", argv[1], argv[2]);
    int err = jdac_validate(argv[1], argv[2]);
    if (err==JDAC_ERR_VALID) {
        printf("validation ok\n");
    } else {
        printf("validate failed %d\n", err);
    }
    return err;
}
