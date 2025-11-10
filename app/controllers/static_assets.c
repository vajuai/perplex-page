#include "../../lavandula/include/lavandula.h"
#include "../../lavandula/include/utils.h"

#include <limits.h>
#include <strings.h>

static const char *DIST_DIR = "frontend/dist";

static const char *guess_mime(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot || dot == path) {
        return TEXT_PLAIN;
    }

    if (strcasecmp(dot, ".html") == 0) {
        return TEXT_HTML;
    }
    if (strcasecmp(dot, ".js") == 0) {
        return "application/javascript";
    }
    if (strcasecmp(dot, ".css") == 0) {
        return "text/css";
    }
    if (strcasecmp(dot, ".wasm") == 0) {
        return "application/wasm";
    }
    if (strcasecmp(dot, ".json") == 0) {
        return APPLICATION_JSON;
    }

    return "application/octet-stream";
}

static bool contains_parent_reference(const char *path) {
    return strstr(path, "..") != NULL;
}

static void trim_query(char *path) {
    for (size_t i = 0; path[i] != '\0'; i++) {
        if (path[i] == '?' || path[i] == '#') {
            path[i] = '\0';
            return;
        }
    }
}

appRoute(static_assets, ctx) {
    const char *resource = ctx.request.resource;
    if (!resource) {
        return notFound(strdup("Not Found"), TEXT_PLAIN);
    }

    char sanitized[PATH_MAX];
    snprintf(sanitized, sizeof(sanitized), "%s", resource);
    trim_query(sanitized);

    const char *relative = sanitized[0] == '/' ? sanitized + 1 : sanitized;

    if (*relative == '\0' || contains_parent_reference(relative)) {
        return notFound(strdup("Not Found"), TEXT_PLAIN);
    }

    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", DIST_DIR, relative);

    FileBuffer buffer = readFileBuffer(fullpath);
    if (!buffer.data) {
        return notFound(strdup("Not Found"), TEXT_PLAIN);
    }

    const char *mime = guess_mime(relative);

    HttpResponse response = ok(buffer.data, (char *)mime);
    response.contentLength = buffer.length;
    return response;
}
