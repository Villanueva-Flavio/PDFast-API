#include "src/config.h"
#include "src/routes.h"

int main() {
    PDFastApp app;
    app.loglevel(static_cast<crow::LogLevel>(config().log_level));
    configure_cors(app);
    setup_routes(app);
    app.bindaddr(config().bind_host).port(static_cast<std::uint16_t>(config().port)).multithreaded().run();
}
