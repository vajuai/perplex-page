#include "../lavandula/src/include/lavandula.h"
#include "controllers/controllers.h"

void registerRoutes(App *app) {
    root(app, htmx_page);
    get(app, "/api/search", search_api);
    get(app, "/htmx", htmx_page);
    get(app, "/htmx/search", htmx_search);
}
