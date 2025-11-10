#include "../../lavandula/include/lavandula.h"
#include <stdbool.h>

#include "../../lavandula/include/utils.h"

static const char *INDEX_PATH = "frontend/dist/index.html";

appRoute(home, ctx) {
    (void)ctx;

    FileBuffer buffer = readFileBuffer(INDEX_PATH);
    if (!buffer.data) {
        const char *msg =
            "Release assets missing.\n"
            "Run `trunk build --release` before starting the server.\n";
        return ok(strdup((char *)msg), TEXT_PLAIN);
    }

    HttpResponse response = ok(buffer.data, TEXT_HTML);
    response.contentLength = buffer.length;
    return response;
}
