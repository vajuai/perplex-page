#include "../../lavandula/include/lavandula.h"
#include <errno.h>
#include <ctype.h>

static char *read_command_output(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        size_t sz = 256;
        char *err = malloc(sz);
        if (!err) return NULL;
        snprintf(err, sz, "Failed to run command: %s", strerror(errno));
        return err;
    }

    size_t cap = 8192; // start with 8KB
    size_t len = 0;
    char *buf = malloc(cap);
    if (!buf) {
        pclose(fp);
        return NULL;
    }

    size_t n;
    char chunk[1024];
    while ((n = fread(chunk, 1, sizeof(chunk), fp)) > 0) {
        if (len + n + 1 > cap) {
            size_t new_cap = cap * 2;
            while (len + n + 1 > new_cap) new_cap *= 2;
            char *tmp = realloc(buf, new_cap);
            if (!tmp) {
                free(buf);
                pclose(fp);
                return NULL;
            }
            cap = new_cap;
            buf = tmp;
        }
        memcpy(buf + len, chunk, n);
        len += n;
    }
    pclose(fp);

    buf[len] = '\0';
    return buf;
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static char *url_decode(const char *src) {
    if (!src) return NULL;
    size_t len = strlen(src);
    char *out = malloc(len + 1);
    if (!out) return NULL;

    size_t oi = 0;
    for (size_t i = 0; i < len; i++) {
        char c = src[i];
        if (c == '+') {
            out[oi++] = ' ';
        } else if (c == '%' && i + 2 < len) {
            int hi = hexval(src[i+1]);
            int lo = hexval(src[i+2]);
            if (hi >= 0 && lo >= 0) {
                out[oi++] = (char)((hi << 4) | lo);
                i += 2;
            } else {
                out[oi++] = c;
            }
        } else {
            out[oi++] = c;
        }
    }
    out[oi] = '\0';
    return out;
}

appRoute(search_api, ctx) {
    // Default query
    char *query_decoded = NULL;

    const char *resource = ctx.request.resource ? ctx.request.resource : "";

    const char *qm = strchr(resource, '?');
    if (qm) {
        const char *params = qm + 1;
        // very simple parsing for q=
        const char *p = strstr(params, "q=");
        if (p) {
            p += 2; // skip q=
            const char *end = strchr(p, '&');
            size_t len = end ? (size_t)(end - p) : strlen(p);
            char *raw = malloc(len + 1);
            if (!raw) return internalServerError(strdup("alloc failed"), TEXT_PLAIN);
            memcpy(raw, p, len); raw[len] = '\0';
            query_decoded = url_decode(raw);
            free(raw);
        }
    }

    const char *query = query_decoded && strlen(query_decoded) > 0 ? query_decoded : "latest technology news";

    const char *api = getenv("PERPLEXITY_API_KEY");
    if (!api || strlen(api) == 0) {
        const char *msg =
            "PERPLEXITY_API_KEY is not set.\n"
            "Set it in your environment and ensure './simpcurl' is executable.\n"
            "Example: export PERPLEXITY_API_KEY=your_key_here\n";
        if (query_decoded) free(query_decoded);
        return unauthorized(strdup((char *)msg), TEXT_PLAIN);
    }

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./simpcurl \"%s\"", query);

    char *output = read_command_output(cmd);
    if (query_decoded) free(query_decoded);

    if (!output) {
        const char *fallback =
            "Failed to retrieve results from Perplexity.\n"
            "Make sure './simpcurl' exists and is executable.\n";
        return serviceUnavailable(strdup((char *)fallback), TEXT_PLAIN);
    }

    return ok(output, TEXT_PLAIN);
}
