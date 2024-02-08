# json-c d'accord library (libjsoncdac)

jsonc-daccord is a JSON Schema validation library written in C, and is taking advantage of the json-c library.

## Design Goals

The goal is to have a lightweight JSON Schema validation implementation in C using json-c.
json-c is popular in OpenWRT communities.

Many of the schema features are made optional, and can be disabled if not needed.

Supported schema keywords:

| object type | feature                                                                   |
| :---------- | :------------------------------------------------------------------------ |
| all         | type, enum, required, properties, const                                   |
| object      | dependentRequired, propertyNames, patternProperties, additionalProperties |
| string      | minLength, maxLength                                                      |
| numbers     | minimum, maximum, multipleOf, exclusiveMinimum, exclusiveMaximum          |
| boolean     | additionalProperties                                                      |
| array       | minItems, maxItems, uniqeItems, items                                     |

Other: $defs, $ref

## Example Use

Public headers:

See [jsoncdaccord.h](include/jsoncdaccord.h)

```C
    int jdac_validate_file(const char *jsonfile, const char *jsonschemafile);
    int jdac_validate(json_object *jobj, json_object *jschema);
    int jdac_ref_set_localpath(const char *_localpath);

    const char* jdac_errorstr(unsigned int jdac_errors);
```

Link your binary to: `-ljsoncdac -ljson-c`

Use the #include header: `#include <jsoncdaccord.h>`

Example C code:

```C
#include <stdio.h>
#include <json-c/json.h>
#include <jsoncdaccord.h>

int main(int argc, char *argv[])
{
    char *json_file = "test.json";
    char *schema_file = "schema.json";

    // optional: load referenced schema files from filesystem
    char *localpath = "/my/path/to_json_files/";
    jdac_ref_set_localpath(localpath);

    printf("validating %s with %s\n", json_file, schema_file);
    int err = jdac_validate_file(json_file, schema_file);
    if (err==JDAC_ERR_VALID) {
        printf("validation ok\n");
    } else {
        printf("validate failed %d: %s\n", err, jdac_errorstr(err));
    }
    return err;
}
```

See [jdac-cli.c](libjsoncdac/jdac-cli.c) as well.

## Install

Building from source:

Install json-c and libcmocka-dev (used in the debug builds).

- Release version:

```
git clone --branch libjsoncdac-0.3 https://github.com/domoslabs/jsonc-daccord &&\
cd jsonc-daccord && mkdir build && cd build &&\
cmake .. -DCMAKE_BUILD_TYPE=Release && make && sudo make install
```

- Debug version:
```
git clone --branch libjsoncdac-0.3 https://github.com/domoslabs/jsonc-daccord &&\
cd jsonc-daccord && mkdir build && cd build &&\
cmake .. -DCMAKE_BUILD_TYPE=Debug && make && sudo make install
```

Note: After install you might need to run `sudo ldconfig`.

## CMake Options

build options:

| option                          | description                                              |
| :------------------------------ | :------------------------------------------------------- |
| CMAKE_BUILD_TYPE                | Build as Release or Debug. Default: Release.             |
| JDAC_RUN_TESTS                  | jdac unit tests (CMAKE_BUILD_TYPE=Debug Only)            |
| JDAC_RUN_TEST_SUITE             | Run json-schema-org's JSON-Schema-Test-Suite             |
| JDAC_USE_ASAN                   | Build with address sanitizer                             |
| JDAC_SHARED_LIBS                | Build .so file, .a otherwise                             |
| JDAC_BUILD_ALL                  | override build options below and build all               |
| JDAC_BUILD_ERROR_OUTPUT         | Show failing validations with neat error messages        |
| JDAC_BUILD_PATTERN              | Support *pattern*.                                       |
| JDAC_BUILD_PATTERNPROPERTIES    | Support *patternProperties*                              |
| JDAC_BUILD_ADDITIONALPROPERTIES | Support *additionalProperties*                           |
| JDAC_BUILD_PROPERTYNAMES        | Support *propertyNames*                                  |
| JDAC_BUILD_SUBSCHEMALOGIC       | Support *allOf*, *anyOf*, *oneOf*, *not*, *if-then-else* |
| JDAC_BUILD_CONTAINS             | Support *contains*, *minContains*, and *maxContains*     |
| JDAC_BUILD_DOWNLOAD             | Support downloading referenced schema files              |
| JDAC_BUILD_STORE                | Support build a list of schema uri, id, and anchors      |
| JDAC_BUILD_REF                  | Support *$ref* keyword. load schemas by file.            |

 Note: Some build options may be off by default. Like DOWNLOAD. Not everyone needs curl+crypto on their system for this.

## Run tests
This should be done before each test
For debug builds:
```
mkdir build
cd build && \
cmake .. -DJDAC_BUILD_ALL=ON -DJDAC_RUN_TESTS=ON -DCMAKE_BUILD_TYPE=debug -DJDAC_USE_ASAN=ON && \
make

ctest
ctest -V # to see output of tests
ctest -R dep -V # to test only output test-dependent
```

To run the test suites, set `RUN_TEST_SUITE=ON` in the cmake options.

## Command Line Interface
You can try the library with the jdac-cli command.

```/tmp/domos/domosqos-sta_statistics_json
jdac-cli -h
```
## To do
- prevent infinite recursion

## Related links

- https://json-schema.org/specification.html
- https://github.com/json-schema-org/JSON-Schema-Test-Suite
- https://github.com/json-c/json-c
