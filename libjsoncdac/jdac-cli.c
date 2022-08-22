#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "../include/jsoncdaccord.h"
#include "version_config.h"

char *json_file=NULL, *schema_file=NULL;

void usage()
{
    printf("\nUsage:\n");
    printf("    jdac-cli -j jsonfile -s schemafile [options]\n\n");
    printf("    options:\n");
#ifdef JDAC_REF
    printf("             --loadlocal, -l   referenced schemas at local filelocationl\n");
#endif
    printf("             --version, -v     Show version and supported JSON Schema keywords\n");
    printf("             --help, -h        Show this\n\n");
}

void freeall()
{
    if (json_file) free(json_file);
    if (schema_file) free(schema_file);
}

int main(int argc, char *argv[])
{

    int c;
    int digit_optind = 0;

    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"json",      required_argument, 0, 'j'},
            {"schema",    required_argument, 0, 's'},
#ifdef JDAC_REF
            {"loadlocal", required_argument, 0, 'l'},
#endif
            {"version",   no_argument,       0, 'v'},
            {"help",      no_argument,       0, 'h'},
            {0,           0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "j:s:l:v",
                long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'j':
                json_file = strdup(optarg);
                break;
            case 's':
                schema_file = strdup(optarg);
                break;
#ifdef JDAC_REF
            case 'l':
                jdac_ref_set_localpath(optarg);
                break;
#endif
            case 'v':
                printf("jdac-cli (%s version %s)\n", PROJECT_NAME, PROJECT_VER);
                printf("supported keywords:\n");
                printf(" - base:     %s\n", SUPPORTED_KEYWORDS_BASE);
                printf(" - selected: %s\n\n", SUPPORTED_KEYWORDS_OPTIONAL);
                freeall();
                return 0;
                break;
            case 'h':
                usage();
                freeall();
                return JDAC_ERR_VALID;
            default:
                printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (!json_file) {
        printf("No json file given\n");
    }

    if (!schema_file) {
        printf("No schema file given");
    }

    if (!json_file || !schema_file) {
        usage();
        freeall();
        return JDAC_ERR_WRONG_ARGS;
    }


    printf("validating %s with %s\n", json_file, schema_file);
    int err = jdac_validate_file(json_file, schema_file);
    if (err==JDAC_ERR_VALID) {
        printf("validation ok\n");
    } else {
        printf("validate failed. err %d: %s\n", err, jdac_errorstr(err));
    }
    freeall();
    return err;
}
