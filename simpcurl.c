#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "yyjson.h"

/*
 Embed yyjson implementation so that building with a single compilation unit works:
   gcc simpcurl.c -lcurl -o simpcurl
 If you prefer to compile yyjson separately, define SIMPCURL_NO_EMBED_YYJSON
 and build like this instead:
   gcc simpcurl.c yyjson.c -lcurl -o simpcurl
 Do NOT compile yyjson.c when embedding it below, to avoid duplicate symbols.
*/
#ifndef SIMPCURL_NO_EMBED_YYJSON
#include "yyjson.c"
#endif

// 结构体，用于在内存中保存curl的响应
struct memory {
    char *response;
    size_t size;
};

// curlopt_writefunction的回调函数
static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct memory *mem = (struct memory *)userp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if(!ptr) {
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}


int main(int c, char **v) {
    char b[1024], a[512];
    CURL *x = curl_easy_init();
    struct curl_slist *h = curl_slist_append(NULL, "Content-Type: application/json");

    // 为响应数据初始化内存块
    struct memory chunk;
    chunk.response = malloc(1);
    chunk.size = 0;

    snprintf(b, sizeof(b), "{\"query\":\"%s\",\"max_results\":3}", v[1]);
    snprintf(a, sizeof(a), "Authorization: Bearer %s", getenv("PERPLEXITY_API_KEY"));
    curl_easy_setopt(x, CURLOPT_URL, "https://api.perplexity.ai/search");
    curl_easy_setopt(x, CURLOPT_POSTFIELDS, b);
    curl_easy_setopt(x, CURLOPT_HTTPHEADER, curl_slist_append(h, a));

    // 设置写回调函数，将响应写入内存
    curl_easy_setopt(x, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(x, CURLOPT_WRITEDATA, (void *)&chunk);

    CURLcode res = curl_easy_perform(x);

    if (res == CURLE_OK) {
        // 使用yyjson解析响应
        yyjson_doc *doc = yyjson_read(chunk.response, chunk.size, 0);
        if (doc) {
            yyjson_val *root = yyjson_doc_get_root(doc);
            yyjson_val *results = yyjson_obj_get(root, "results");

            if (results && yyjson_is_arr(results)) {
                yyjson_val *result;
                yyjson_arr_iter iter;
                yyjson_arr_iter_init(results, &iter);

                printf("Search Results:\n");
                printf("-----------------\n");

                int i = 1;
                // 遍历 'results' 数组，限制最多输出3条
                while ((result = yyjson_arr_iter_next(&iter)) && i <= 3) {
                    yyjson_val *title_val = yyjson_obj_get(result, "title");
                    yyjson_val *url_val = yyjson_obj_get(result, "url");
                    yyjson_val *snippet_val = yyjson_obj_get(result, "snippet");

                    const char *title = yyjson_get_str(title_val);
                    const char *url = yyjson_get_str(url_val);
                    const char *snippet = yyjson_get_str(snippet_val);

                    printf("Result %d:\n", i++);
                    if (title) printf("  Title: %s\n", title);
                    if (url) printf("  URL: %s\n", url);
                    if (snippet) printf("  Snippet: %s\n", snippet);
                    printf("\n");
                }
            } else {
                 printf("Could not find 'results' array in JSON response.\n");
            }
            
            // 释放yyjson文档
            yyjson_doc_free(doc);
        } else {
            printf("Failed to parse JSON.\n");
        }
    } else {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    }

    // 释放内存和curl句柄
    free(chunk.response);
    curl_slist_free_all(h), curl_easy_cleanup(x);
}
