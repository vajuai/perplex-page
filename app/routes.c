#include "../lavandula/include/lavandula.h"
#include "controllers/controllers.h"

void registerRoutes(App *app) {
    root(app, home);
    get(app, "/api/search", search_api);
    get(app, "/htmx", htmx_page);
    get(app, "/htmx/search", htmx_search);
    routeNotFound(app, static_assets);
}
