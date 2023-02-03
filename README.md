# json-c d'accord library (libjsoncdac)

jsonc-daccord is a lightweight JSON Schema validation library written in C, and is taking advantage of the libjson-c library.

## Design Goals

The goal is to have a lightweight JSON Schema validation implementation in C using json-c. json-c is popular in OpenWRT communities. Initially I just wanted it to support a small subset of JSON Schema to suit a need to validate simple json files. See the minimum build supports below. However to suit a broader audience, supporting more JSON Schema is important.

Currently the footprint of libjsoncdac.so is 8KB. The keep the footprint from bloating out, new features should be selectable using CMake options.

Minimal build supports:
- all: type, enum, required, properties, const.
- objects: 
- strings: minLength, maxLength.
- integers and doubles: minimum, maximum.
- arrays: minItems, maxItems, uniqeItems, items.

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
git clone --branch libjsoncdac-0.2 https://github.com/domoslabs/jsonc-daccord &&\
cd jsonc-daccord && mkdir build && cd build &&\
cmake .. -DCMAKE_BUILD_TYPE=Release && make && sudo make install
```

- Debug version:
```
git clone --branch libjsoncdac-0.2 https://github.com/domoslabs/jsonc-daccord &&\
cd jsonc-daccord && mkdir build && cd build &&\
cmake .. -DCMAKE_BUILD_TYPE=Debug && make && sudo make install
```

Note: After install you might need to run `sudo ldconfig`.

## CMake Options

build options:

| option                     | description                                              |
| :------------------------- | :------------------------------------------------------- |
| CMAKE_BUILD_TYPE           | Build as Release or Debug. Default: Release.             |
| RUN_TEST_SUITE             | Run JSON Schema test suite (CMAKE_BUILD_TYPE=Debug Only) |
| BUILD_PATTERN              | Support *pattern*.                                       |
| BUILD_PATTERNPROPERTIES    | Support *patternProperties*                              |
| BUILD_ADDITIONALPROPERTIES | Support *additionalProperties*                           |
| BUILD_PROPERTYNAMES        | Support *propertyNames*                                  |
| BUILD_SUBSCHEMALOGIC       | Support *allOf*, *anyOf*, *oneOf*, *not*, *if-then-else* |
| BUILD_CONTAINS             | Support *contains*, *minContains*, and *maxContains*     |
| BUILD_DOWNLOAD             | Support downloading referenced schema files              |
| BUILD_STORE                | Support build a list of schema uri, id, and anchors      |
| BUILD_REF                  | Support *$ref* keyword. load schemas by file.            |

 Note: Some build options may be off by default. Like DOWNLOAD. Not everyone needs curl+crypto on their system for this.

## Run tests
For debug builds:
```
ctest
ctest -V # to see output of tests
```

Running test suites are currently optional, and are select with `RUN_TEST_SUITE=ON` in the cmake options.

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
