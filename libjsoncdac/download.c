#include <stdio.h>
#include <string.h>
#include <json-c/json.h>
#include <curl/curl.h>
#include "../include/jsoncdaccord.h"
#include "../include/jdac_internal.h"
#include "../include/jdac_optional.h"

struct curlmemory {
    char *response;
    size_t size;
};
 
static size_t cb(void *data, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct curlmemory *mem = (struct curlmemory *)userp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(ptr == NULL)
        return 0;  /* out of memory! */

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}
 
char* _jdac_download_schema(const char *url)
{
    struct curlmemory chunk = {0};
 
    CURL *curl = curl_easy_init();
    if(curl) {
        CURLcode res;
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));
        curl_easy_cleanup(curl);
    }
    return chunk.response;
}

const char* _jdac_download_resolve(const char *uri)
{
    int len = strlen(uri);
    if (len > 8 && strncmp(uri, "http", 4)==0) {
        return uri;
    }
    return NULL;
}
