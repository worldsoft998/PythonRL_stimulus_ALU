/* cJSON.h - minimal embedded version for PoC
 * Only the API used by dpi_socket.c is exposed.
 * This is a tiny adapted header for embedding.
 */

#ifndef CJSON_MIN_H
#define CJSON_MIN_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct cJSON {
    struct cJSON *next;
    struct cJSON *prev;
    struct cJSON *child;
    int type;
    char *valuestring;
    int valueint;
    double valuedouble;
    char *string;
} cJSON;

cJSON* cJSON_Parse(const char *value);
void cJSON_Delete(cJSON *c);
cJSON* cJSON_GetObjectItemCaseSensitive(cJSON *const object, const char *const string);
int cJSON_IsArray(cJSON *item);
int cJSON_IsString(cJSON *item);
int cJSON_IsNumber(cJSON *item);

#define cJSON_IsString(item) ((item)!=(NULL) && ((item)->type==1))
#define cJSON_IsNumber(item) ((item)!=(NULL) && ((item)->type==2))
#define cJSON_IsArray(item) ((item)!=(NULL) && ((item)->type==3))

/* helpers for iteration */
#define cJSON_ArrayForEach(element, array)     for (element = (array ? (array)->child : NULL); element != NULL; element = element->next)

#ifdef __cplusplus
}
#endif

#endif /* CJSON_MIN_H */
