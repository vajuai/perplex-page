#include "../../lavandula/src/include/lavandula.h"

#include <stdio.h>
#include <stdlib.h>

static const char *HTMX_INDEX = "frontend_htmx/index.html";

static char *read_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return NULL;
    }

    long size = ftell(fp);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }
    rewind(fp);

    char *buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(fp);
        return NULL;
    }

    size_t read = fread(buffer, 1, (size_t)size, fp);
    fclose(fp);
    if (read != (size_t)size) {
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';
    return buffer;
}

appRoute(htmx_page, ctx) {
    (void)ctx;

    char *html = read_file(HTMX_INDEX);
    if (!html) {
        const char *msg =
            "HTMX frontend not found.\n"
            "Ensure frontend_htmx/index.html exists and is included in deployments.\n";
        return notFound(strdup((char *)msg), TEXT_PLAIN);
    }

    return ok(html, TEXT_HTML);
}
