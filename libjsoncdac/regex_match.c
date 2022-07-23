#include <regex.h>
#include <stdio.h>
#include <string.h>
#include "../include/jsoncdaccord.h"

int _jdac_match_string_with_regex(const char* regex_pattern, const char* value)
{
    regex_t regex;
    int reti = regcomp(&regex, regex_pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return JDAC_REGEX_COMPILE_FAILED;
    }
    reti = regexec(&regex, value, 0, NULL, 0);
    regfree(&regex);
    if (reti==0) {
         return JDAC_REGEX_MATCH;
    }
    return JDAC_REGEX_MISMATCH;
}
