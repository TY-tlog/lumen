// Lumen — entry point.

#include "app/Application.h"

#include <QtGlobal>

void initResources()
{
    Q_INIT_RESOURCE(styles);
    Q_INIT_RESOURCE(fonts);
}

int main(int argc, char* argv[]) {
    initResources();
    lumen::Application app(argc, argv);
    return app.run();
}
