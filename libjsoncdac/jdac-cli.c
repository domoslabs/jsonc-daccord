#include <stdio.h>
#include <string.h>
#include "../include/jsoncdaccord.h"
#include "version_config.h"

void usage()
{
    printf("\nUsage:\n");
    printf("    jdac-cli jsonfile schemafile    Validate json with schema\n");
    printf("    jdac-cli --version              Show version and supported JSON Schema keywords\n\n");
}

int main(int argc, char *argv[])
{

    if (argc==2 && strncmp("--version", argv[1], 11)==0) {
        printf("jdac-cli (%s version %s)\n", PROJECT_NAME, PROJECT_VER);
        printf("supported keywords:\n");
        printf(" - base:     %s\n", SUPPORTED_KEYWORDS_BASE);
        printf(" - selected: %s\n\n", SUPPORTED_KEYWORDS_OPTIONAL);
        return 0;
    }

    if (argc != 3) {
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
