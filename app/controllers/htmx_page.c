#include "../../lavandula/include/lavandula.h"
#include "../../lavandula/include/utils.h"

static const char *HTMX_INDEX = "frontend_htmx/index.html";

appRoute(htmx_page, ctx) {
    (void)ctx;

    FileBuffer buffer = readFileBuffer(HTMX_INDEX);
    if (!buffer.data) {
        const char *msg =
            "HTMX frontend not found.\n"
            "Ensure frontend_htmx/index.html exists and is included in deployments.\n";
        return notFound(strdup((char *)msg), TEXT_PLAIN);
    }

    HttpResponse response = ok(buffer.data, TEXT_HTML);
    response.contentLength = buffer.length;
    return response;
}
