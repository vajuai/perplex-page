#include "../lavandula/include/lavandula.h"
#include "controllers/controllers.h"

void registerRoutes(App *app) {
    root(app, home);
    get(app, "/api/search", search_api);
}
