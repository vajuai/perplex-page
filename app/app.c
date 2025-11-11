#include "../lavandula/src/include/lavandula.h"
#include "routes.h"

int main() {
    AppBuilder builder = createBuilder();

    // Enable permissive CORS so the Yew frontend (served by Trunk on a different port) can call our API
    useDefaultCorsPolicy(&builder);

    App app = build(builder);

    registerRoutes(&app);

    runApp(&app);

    return 0;
}
