/* cJSON.c - tiny embedded parser for PoC
 * NOTE: This is a deliberately tiny and not fully-featured parser.
 * It supports parsing arrays of objects containing simple string keys and numeric values.
 * For production, use upstream cJSON library.
 */

#include "cJSON.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char *skip_ws(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

/* Very small helpers that recognize JSON tokens we expect.
   This does not implement full JSON; it is tailored to arrays of objects
   like: [{"op":"add","a":12,"b":34}, ...] */
static char *find_next(char *s, char c) {
    while (*s && *s != c) {
        if (*s == '\"') {
            s++;
            while (*s && *s != '\"') {
                if (*s == '\\' && *(s+1)) s+=2; else s++;
            }
            if (*s == '\"') s++;
        } else s++;
    }
    return *s ? s : NULL;
}

/* Simple strdup */
static char *sdup(const char *s) {
    if (!s) return NULL;
    char *d = malloc(strlen(s)+1);
    if (d) strcpy(d, s);
    return d;
}

/* Create a minimal cJSON object */
static cJSON* new_item(void) {
    cJSON *it = (cJSON*)malloc(sizeof(cJSON));
    if (!it) return NULL;
    memset(it,0,sizeof(cJSON));
    return it;
}

/* A tiny parser that extracts an array of shallow objects.
   For each object we create a cJSON node with child nodes (string keys or numbers).
*/
cJSON* cJSON_Parse(const char *value) {
    if (!value) return NULL;
    char *s = (char*)value;
    s = skip_ws(s);
    if (*s != '[') return NULL;
    s++;
    cJSON *root = new_item();
    root->type = 3; // array
    cJSON *last = NULL;
    while (1) {
        s = skip_ws(s);
        if (*s == ']') break;
        if (*s == '{') {
            s++;
            cJSON *obj = new_item();
            obj->type = 4; // object
            obj->child = NULL;
            cJSON *lastchild = NULL;
            while (1) {
                s = skip_ws(s);
                if (*s == '}') { s++; break; }
                if (*s != '\"') break;
                s++;
                char *keystart = s;
                while (*s && *s != '\"') {
                    if (*s == '\\' && *(s+1)) s+=2; else s++;
                }
                size_t keylen = s - keystart;
                char *key = malloc(keylen+1);
                strncpy(key, keystart, keylen); key[keylen]=0;
                if (*s == '\"') s++;
                s = skip_ws(s);
                if (*s == ':') s++;
                s = skip_ws(s);
                /* value: string or number */
                if (*s == '\"') {
                    s++;
                    char *vstart = s;
                    while (*s && *s != '\"') {
                        if (*s == '\\' && *(s+1)) s+=2; else s++;
                    }
                    size_t vlen = s - vstart;
                    char *valstr = malloc(vlen+1);
                    strncpy(valstr, vstart, vlen); valstr[vlen]=0;
                    if (*s == '\"') s++;
                    cJSON *child = new_item();
                    child->type = 1;
                    child->string = key;
                    child->valuestring = valstr;
                    if (!obj->child) obj->child = child; else lastchild->next = child;
                    lastchild = child;
                } else {
                    /* number scanning */
                    char *nstart = s;
                    int neg = 0;
                    if (*s == '-') { neg = 1; s++; }
                    while (isdigit((unsigned char)*s)) s++;
                    int intval = atoi(nstart);
                    cJSON *child = new_item();
                    child->type = 2;
                    child->string = key;
                    child->valueint = intval;
                    if (!obj->child) obj->child = child; else lastchild->next = child;
                    lastchild = child;
                }
                s = skip_ws(s);
                if (*s == ',') { s++; continue; } else if (*s == '}') { s++; break; } else break;
            }
            /* append obj to root->child list */
            if (!root->child) root->child = obj; else last->next = obj;
            last = obj;
            s = skip_ws(s);
            if (*s == ',') { s++; continue; } else if (*s == ']') break;
        } else break;
    }
    return root;
}

/* Find object item by key */
cJSON* cJSON_GetObjectItemCaseSensitive(cJSON *const object, const char *const string) {
    if (!object || !object->child) return NULL;
    cJSON *c = object->child;
    while (c) {
        if (c->string && strcmp(c->string, string) == 0) return c;
        c = c->next;
    }
    return NULL;
}

void cJSON_Delete(cJSON *c) {
    if (!c) return;
    /* recursively free */
    if (c->child) {
        cJSON *ch = c->child;
        while (ch) {
            cJSON *n = ch->next;
            cJSON_Delete(ch);
            ch = n;
        }
    }
    if (c->valuestring) free(c->valuestring);
    if (c->string) free(c->string);
    free(c);
}
