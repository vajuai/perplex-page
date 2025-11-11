#include "../../lavandula/include/lavandula.h"
#include <errno.h>
#include <ctype.h>

typedef struct {
    char *query;
    bool  owns;
} QueryInfo;

static char *read_command_output(const char *cmd) {
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        size_t sz = 256;
        char *err = malloc(sz);
        if (!err) return NULL;
        snprintf(err, sz, "Failed to run command: %s", strerror(errno));
        return err;
    }

    size_t cap = 8192;
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

static QueryInfo extract_query(RequestContext ctx) {
    QueryInfo info = {
        .query = "latest technology news",
        .owns = false
    };

    const char *resource = ctx.request.resource ? ctx.request.resource : "";
    const char *qm = strchr(resource, '?');
    if (!qm) return info;

    const char *params = qm + 1;
    const char *p = strstr(params, "q=");
    if (!p) return info;

    p += 2;
    const char *end = strchr(p, '&');
    size_t len = end ? (size_t)(end - p) : strlen(p);
    char *raw = malloc(len + 1);
    if (!raw) return info;
    memcpy(raw, p, len);
    raw[len] = '\0';

    char *decoded = url_decode(raw);
    free(raw);
    if (decoded && strlen(decoded) > 0) {
        info.query = decoded;
        info.owns = true;
    } else if (decoded) {
        free(decoded);
    }

    return info;
}

static char *html_escape(const char *input) {
    size_t len = strlen(input);
    size_t cap = len * 5 + 1;
    char *out = malloc(cap);
    if (!out) return NULL;

    size_t idx = 0;
    for (size_t i = 0; i < len; i++) {
        char c = input[i];
        const char *rep = NULL;
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            default: break;
        }

        if (rep) {
            size_t rlen = strlen(rep);
            if (idx + rlen >= cap) {
                cap = cap * 2 + rlen;
                char *tmp = realloc(out, cap);
                if (!tmp) {
                    free(out);
                    return NULL;
                }
                out = tmp;
            }
            memcpy(out + idx, rep, rlen);
            idx += rlen;
        } else {
            if (idx + 1 >= cap) {
                cap = cap * 2 + 1;
                char *tmp = realloc(out, cap);
                if (!tmp) {
                    free(out);
                    return NULL;
                }
                out = tmp;
            }
            out[idx++] = c;
        }
    }
    out[idx] = '\0';
    return out;
}

static HttpResponse build_search_response(const char *query, bool as_html) {
    const char *api = getenv("PERPLEXITY_API_KEY");
    if (!api || strlen(api) == 0) {
        const char *msg =
            "PERPLEXITY_API_KEY is not set.\n"
            "Set it in your environment and ensure './simpcurl' is executable.\n"
            "Example: export PERPLEXITY_API_KEY=your_key_here\n";
        if (as_html) {
            char *escaped = html_escape(msg);
            if (!escaped) return internalServerError(strdup("alloc failed"), TEXT_PLAIN);
            size_t needed = strlen(escaped) + 64;
            char *html = malloc(needed);
            if (!html) {
                free(escaped);
                return internalServerError(strdup("alloc failed"), TEXT_PLAIN);
            }
            snprintf(html, needed, "<div class=\"error\">%s</div>", escaped);
            free(escaped);
            HttpResponse resp = unauthorized(html, TEXT_HTML);
            resp.contentLength = strlen(html);
            return resp;
        }
        return unauthorized(strdup((char *)msg), TEXT_PLAIN);
    }

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "./simpcurl \"%s\"", query);

    char *output = read_command_output(cmd);
    if (!output) {
        const char *fallback =
            "Failed to retrieve results from Perplexity.\n"
            "Make sure './simpcurl' exists and is executable.\n";
        if (as_html) {
            char *escaped = html_escape(fallback);
            if (!escaped) return internalServerError(strdup("alloc failed"), TEXT_PLAIN);
            size_t needed = strlen(escaped) + 64;
            char *html = malloc(needed);
            if (!html) {
                free(escaped);
                return internalServerError(strdup("alloc failed"), TEXT_PLAIN);
            }
            snprintf(html, needed, "<div class=\"error\">%s</div>", escaped);
            free(escaped);
            HttpResponse resp = serviceUnavailable(html, TEXT_HTML);
            resp.contentLength = strlen(html);
            return resp;
        }
        return serviceUnavailable(strdup((char *)fallback), TEXT_PLAIN);
    }

    if (!as_html) {
        HttpResponse resp = ok(output, TEXT_PLAIN);
        resp.contentLength = strlen(output);
        return resp;
    }

    char *escaped = html_escape(output);
    free(output);
    if (!escaped) {
        return internalServerError(strdup("alloc failed"), TEXT_PLAIN);
    }

    const char *template =
        "<pre class=\"result-block\">%s</pre>";
    size_t needed = strlen(template) + strlen(escaped);
    char *html = malloc(needed + 1);
    if (!html) {
        free(escaped);
        return internalServerError(strdup("alloc failed"), TEXT_PLAIN);
    }
    snprintf(html, needed + 1, template, escaped);
    free(escaped);

    HttpResponse resp = ok(html, TEXT_HTML);
    resp.contentLength = strlen(html);
    return resp;
}

appRoute(search_api, ctx) {
    QueryInfo info = extract_query(ctx);
    HttpResponse resp = build_search_response(info.query, false);
    if (info.owns && info.query) {
        free(info.query);
    }
    return resp;
}

appRoute(htmx_search, ctx) {
    QueryInfo info = extract_query(ctx);
    HttpResponse resp = build_search_response(info.query, true);
    if (info.owns && info.query) {
        free(info.query);
    }
    return resp;
}
