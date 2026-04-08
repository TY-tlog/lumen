// Lumen — entry point.
//
// Phase 0: opens an empty main window. Future phases add real functionality.

#include "app/Application.h"

int main(int argc, char* argv[]) {
    lumen::Application app(argc, argv);
    return app.run();
}
