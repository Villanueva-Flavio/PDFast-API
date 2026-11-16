#ifndef PDFAST_ROUTES_H
#define PDFAST_ROUTES_H

#include "crow.h"
#include "crow/middlewares/cors.h"

using PDFastApp = crow::App<crow::CORSHandler>;

/**
 * @brief Configures the CORS middleware from the configuration.
 * @param app The application instance
 **/
void configure_cors(PDFastApp& app);

/**
 * @brief Registers every HTTP route of the API.
 * @param app The application instance
 **/
void setup_routes(PDFastApp& app);

#endif
