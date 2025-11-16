// dpi/dpi_socket.c
// DPI-C socket + JSON batch parsing for PoC.
// Compile as C file with VCS, or add -DUSE_CJSON -lcjson to use cJSON library.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#ifdef USE_CJSON
#include <cjson/cJSON.h>
#endif

#define MAX_BUF 65536
#define MAX_ACTIONS 1024

static int sockfd = -1;

// storage for parsed actions after recv+parse
static int action_count = 0;
static int action_op[MAX_ACTIONS];
static int action_a[MAX_ACTIONS];
static int action_b[MAX_ACTIONS];

int dpi_socket_connect(const char *host, int port) {
    struct sockaddr_in serv_addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return -1;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        close(sockfd); sockfd = -1; return -2;
    }
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd); sockfd = -1; return -3;
    }
    return 0;
}

int dpi_socket_close() {
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
        return 0;
    }
    return -1;
}

int dpi_socket_send_json(const char *json_str, int len) {
    if (sockfd < 0) return -1;
    uint32_t be_len = htonl((uint32_t)len);
    ssize_t w;
    w = write(sockfd, &be_len, 4);
    if (w != 4) return -2;
    w = write(sockfd, json_str, len);
    if (w != len) return -3;
    return 0;
}

int dpi_socket_recv_json(char *buf, int buf_len) {
    if (sockfd < 0) return -1;
    uint32_t be_len;
    int r = read(sockfd, &be_len, 4);
    if (r == 0) return -2; // closed
    if (r != 4) return -3;
    uint32_t len = ntohl(be_len);
    if (len + 1 > (uint32_t)buf_len) return -4;
    int total = 0;
    while (total < (int)len) {
        int got = read(sockfd, buf + total, len - total);
        if (got <= 0) return -5;
        total += got;
    }
    buf[total] = '\\0';
    return total;
}

// send batch (just wrapper)
int dpi_send_obs_batch(const char *json_array, int len) {
    return dpi_socket_send_json(json_array, len);
}

// helper: reset action storage
static void reset_actions() {
    action_count = 0;
    memset(action_op, 0, sizeof(action_op));
    memset(action_a, 0, sizeof(action_a));
    memset(action_b, 0, sizeof(action_b));
}

#ifdef USE_CJSON

// parse JSON array of actions: each action { "type":"action","seq_id":N,"op":"add","a":12,"b":34 }
int dpi_recv_actions_parse() {
    char buf[MAX_BUF];
    int r = dpi_socket_recv_json(buf, MAX_BUF);
    if (r <= 0) return -1;
    reset_actions();

    cJSON *root = cJSON_Parse(buf);
    if (!root) return -2;
    if (!cJSON_IsArray(root)) { cJSON_Delete(root); return -3; }

    int idx = 0;
    cJSON *item = NULL;
    cJSON_ArrayForEach(item, root) {
        if (idx >= MAX_ACTIONS) break;
        cJSON *op = cJSON_GetObjectItemCaseSensitive(item, "op");
        cJSON *a = cJSON_GetObjectItemCaseSensitive(item, "a");
        cJSON *b = cJSON_GetObjectItemCaseSensitive(item, "b");
        if (cJSON_IsString(op) && op->valuestring != NULL) {
            if (strcmp(op->valuestring, "add") == 0) action_op[idx] = 0;
            else if (strcmp(op->valuestring, "sub") == 0) action_op[idx] = 1;
            else if (strcmp(op->valuestring, "and") == 0) action_op[idx] = 2;
            else if (strcmp(op->valuestring, "or")  == 0) action_op[idx] = 3;
            else if (strcmp(op->valuestring, "xor") == 0) action_op[idx] = 4;
            else action_op[idx] = 0;
        } else action_op[idx] = 0;
        action_a[idx] = cJSON_IsNumber(a) ? a->valueint : 0;
        action_b[idx] = cJSON_IsNumber(b) ? b->valueint : 0;
        idx++;
    }
    action_count = idx;
    cJSON_Delete(root);
    return action_count;
}

#else

// Fallback simple parser for expected format: JSON array of objects where each contains "op":"xxx","a":N,"b":M
// This is not a full JSON parser but robust enough for PoC where server sends neat JSON.
int dpi_recv_actions_parse() {
    char buf[MAX_BUF];
    int r = dpi_socket_recv_json(buf, MAX_BUF);
    if (r <= 0) return -1;
    reset_actions();

    // find each '{' ... '}'
    char *p = buf;
    int idx = 0;
    while ((p = strchr(p, '{')) != NULL && idx < MAX_ACTIONS) {
        char *q = strchr(p, '}');
        if (!q) break;
        int len = q - p + 1;
        char chunk[1024];
        if (len >= (int)sizeof(chunk)) { p = q+1; continue; }
        memcpy(chunk, p, len); chunk[len] = '\\0';
        // find op
        if (strstr(chunk, "\"add\"") || strstr(chunk, "add")) action_op[idx] = 0;
        else if (strstr(chunk, "\"sub\"") || strstr(chunk, "sub")) action_op[idx] = 1;
        else if (strstr(chunk, "\"and\"") || strstr(chunk, "and")) action_op[idx] = 2;
        else if (strstr(chunk, "\"or\"")  || strstr(chunk, "or"))  action_op[idx] = 3;
        else if (strstr(chunk, "\"xor\"") || strstr(chunk, "xor")) action_op[idx] = 4;
        else action_op[idx] = 0;
        // find a and b via sscanf
        int aa=-1, bb=-1;
        char *pa = strstr(chunk, "\"a\"");
        if (!pa) pa = strstr(chunk, "a");
        if (pa) {
            // find first number after 'a'
            int n = sscanf(pa, "%*[^0-9-]%d", &aa);
            if (n != 1) aa = 0;
        } else aa = 0;
        char *pb = strstr(chunk, "\"b\"");
        if (!pb) pb = strstr(chunk, "b");
        if (pb) {
            int n = sscanf(pb, "%*[^0-9-]%d", &bb);
            if (n != 1) bb = 0;
        } else bb = 0;
        action_a[idx] = aa;
        action_b[idx] = bb;
        idx++;
        p = q + 1;
    }
    action_count = idx;
    return action_count;
}
#endif

int dpi_get_action_count() { return action_count; }
int dpi_get_action_op(int idx) { if (idx < 0 || idx >= action_count) return 0; return action_op[idx]; }
int dpi_get_action_a(int idx)  { if (idx < 0 || idx >= action_count) return 0; return action_a[idx]; }
int dpi_get_action_b(int idx)  { if (idx < 0 || idx >= action_count) return 0; return action_b[idx]; }
